#define _CRT_SECURE_NO_WARNINGS 1
//#undef _WIN32
/*  #########################################################################  */

#include "StdAfx.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <errno.h>
#ifdef _WIN32
#include <tchar.h>
#else
//#define _stprintf sprintf

//#define _tsystem system
#endif
//#define _stprintf swprintf_s

#include "../cpp/Common/MyString.h"
#include "../cpp/Common/IntToString.h"
#include "../cpp/Common/StringConvert.h"
#include "../cpp/Common/Wildcard.h"
#include "../cpp/Common/CommandLineParser.h"
#include "../cpp/Common/ListFileUtils.h"
#include "../cpp/Common/StdOutStream.h"
#include "../cpp/Windows/FileFind.h"
#include "../cpp/Windows/FileDir.h"
#include "../cpp/Windows/FileIO.h"
#include "../cpp/Windows/DLL.h"
#include "../cpp/7zip/Archive/7z/7zHandler.h"
#include "../cpp/7zip/UI/Common/ExitCode.h"
#include "../cpp/7zip/MyVersion.h"


extern "C"
{
    #include "../c/7zCrc.h"
}

#include "t7z.h"

/*  #########################################################################  */

#ifdef _WIN32
#if !defined(UNICODE) || !defined(_UNICODE)
    #error you're trying to compile t7z without unicode support, do not do this
    //if you do, be sure to change signature, as it will produce different results in certain cases
#endif
#endif

/*  #########################################################################  */

static const char *k7zCopyrightString = "7-Zip"
#ifndef EXTERNAL_CODECS
" (A)"
#endif

#ifdef _WIN64
" [64]"
#endif

" " MY_VERSION_COPYRIGHT_DATE;

extern int MY_CDECL main3 // change to proper name (7zip main)
(
    //#ifndef _WIN32
    //int numArguments,const char*arguments[]
    //#endif
    UStringVector commandStrings
);
//variable is internal to 7zip atually
//Note: DON"T USE FOR RECURSION
extern bool g_CaseSensitive;

/*  #########################################################################  */

namespace torrent7z
{

/*  #########################################################################  */

struct finfo
{
    NWindows::NFile::NFind::CFileInfo fileInfo;
    UInt32 fcount;
    UInt32 dcount;
    UInt32 tcount;
    UInt64 ttl_fs;
    UInt64 max_fs;
    UInt64 min_fs;
    UInt64 avg_fs;
    bool debugprint;
    CSysStringVector*dirlist;
    CSysStringVector*filelist;
};

struct cfinfo
{
    UInt32 fcountt;
    UInt32 fcount;
    UInt32 fcountp;
    UInt32 fcounte;
    UInt32 fcountr;
};

typedef int (*file_proc)(const NWindows::NFile::NFind::CFileInfo&fileInfo,const CSysString&,const CSysString&,void*);

#define fix(x,a) ((a)*(((x)+(a)-1)/(a)))

const int crcsz=128;

//Can't get rid of these globals yet. For the time being, converting them to arrays to allow recursion
// sort fo like a stack
#define MAX_RECURSIONS 5
unsigned short recursion_id = 0;
bool g_stripFileNames[MAX_RECURSIONS];
bool g_singleFile[MAX_RECURSIONS];
bool g_forceRecompress[MAX_RECURSIONS];
bool g_noninteractive[MAX_RECURSIONS];
bool g_IsParentGui[MAX_RECURSIONS];
bool g_firstInstance[MAX_RECURSIONS];
bool g_defaultPriority[MAX_RECURSIONS];
bool g_isDeleteOp[MAX_RECURSIONS];
bool g_keepnonsolid[MAX_RECURSIONS];
bool g_yplus[MAX_RECURSIONS];
bool g_nplus[MAX_RECURSIONS];
bool g_nocopyright[MAX_RECURSIONS];
bool g_createnonsolid[MAX_RECURSIONS];
bool g_createnonsolid_r[MAX_RECURSIONS];
//Shifting to local space
//CSysString addcmds(text(""));

//retaining these as global, don't foresee internal recursions setting them
UINT codePage;
CSysString logFileName;
//CSysString t7z_exe,e7z_exe;

//BITT: removed this global variable. It's passed through funtion arguments instead
//char*buffer;

#ifdef _WIN32
const TCHAR dirDelimiter0='\\';
const TCHAR dirDelimiter1='/';
#else
const TCHAR dirDelimiter0='/';
const TCHAR dirDelimiter1='/';
#endif

const bool cmpro_wa_0=false;
const bool cmpro_wa_1=true && (!cmpro_wa_0);
const bool cmpro_wa=cmpro_wa_0||cmpro_wa_1;


struct
{
    TCHAR*ext;
    int a;
} knownext[]={{text(".7z"),1},{text(".zip"),0},{text(".rar"),0},{0,0}};

/*  #########################################################################  */

static inline UINT GetCurrentCodePage()
{
#ifdef _WIN32
    return ::AreFileApisANSI()?CP_ACP:CP_OEMCP;
#else
    return CP_OEMCP;
#endif
}

/*  #########################################################################  */

#ifndef _UNICODE
AString u2a(const UString&u)
{
    bool tmp;
    return UnicodeStringToMultiByte(u,GetCurrentCodePage(),'?',tmp);
}

UString a2u(const AString&a)
{
    return MultiByteToUnicodeString(a,GetCurrentCodePage());
}
#endif

/*  #########################################################################  */

#ifdef _WIN32
    #define setenv(a,b,c) SetEnvironmentVariable(a,b)
#endif

/*  #########################################################################  */

bool stripFileNames()
{
    return g_stripFileNames[recursion_id];
}

int t7z_main(UStringVector commandStrings, char *buffer);
/*  #########################################################################  */

bool isDeleteOp()
{
    return g_isDeleteOp[recursion_id];
}

//just used to store some default values into all versions of the switches
void initialize_global_switches(){
    for (unsigned short i =0; i < MAX_RECURSIONS; i++){
        g_createnonsolid[i]=false;
        g_createnonsolid_r[i]=false; 
        g_singleFile[i]=false;
        g_stripFileNames[i]=false; 
        g_IsParentGui[i] = false;
        g_forceRecompress[i]=false;
        g_noninteractive[i]=false;
        g_firstInstance[i]=false;
        g_defaultPriority[i]=false;    
        g_isDeleteOp[i]=false;
        g_keepnonsolid[i]=false;    
        g_yplus[i]=false;
        g_nplus[i]=false;    
        g_nocopyright[i]=false;
    }
}

/*  #########################################################################  */

const char*get_t7zsig()
{
    static char _t7zsig[t7zsig_size];
    memcpy(_t7zsig,t7zsig,t7zsig_size);
#ifdef _UNICODE
    _t7zsig[16]=1;
#else
    _t7zsig[16]=0;
#endif
    _t7zsig[16]|=g_singleFile[recursion_id]?2:0;
    _t7zsig[16]|=g_stripFileNames[recursion_id]?4:0;
    return _t7zsig;
}

/*  #########################################################################  */

bool compare_t7zsig(const char*fdata)
{
    char _t7zsig[t7zsig_size];
    memcpy(_t7zsig,fdata,t7zsig_size);
    if(_t7zsig[16]&(~7))
    {
        return 0;
    }
#ifdef _UNICODE
    if(!(_t7zsig[16]&1))
#else
    if(_t7zsig[16]&1)
#endif
    {
        return 0;
    }
    if(_t7zsig[16]&2)
    {
        bool a=g_stripFileNames[recursion_id];
        bool b=(_t7zsig[16]&4)!=0;
        if(a!=b)
        {
            return 0;
        }
    }
    else
    {
        if(_t7zsig[16]&4)
        {
            return 0;
        }
    }
    _t7zsig[16]=0;
    return memcmp(_t7zsig,t7zsig,t7zsig_size)==0;
}

/*  #########################################################################  */

TCHAR*_zt(const CSysString&str)
{
    const TCHAR*zt=str;
    return const_cast<TCHAR*>(zt);
}

/*  #########################################################################  */

int strcmpi_x(const UString&str0,const UString&str1)
{
    UString str=str0.Left(str1.Length());
    return str.CompareNoCase(str1);
}

/*  #########################################################################  */

int is_command(const UString&str)
{
    if(str.CompareNoCase(L"a")==0)return 1;
    if(str.CompareNoCase(L"b")==0)return 1;
    if(str.CompareNoCase(L"d")==0)return 1;
    if(str.CompareNoCase(L"e")==0)return 1;
    if(str.CompareNoCase(L"l")==0)return 1;
    if(str.CompareNoCase(L"t")==0)return 1;
    if(str.CompareNoCase(L"u")==0)return 1;
    if(str.CompareNoCase(L"x")==0)return 1;
    return 0;
}

/*  #########################################################################  */
// checks if  the input string was  command string i.e. -a -ab etc
int is_switch(const UString&str)
{
    if(str[0]=='-')return 1;
    return 0;
}

/*  #########################################################################  */

int is_allowed(const UString&str)
{
    if(str.CompareNoCase(L"e")==0)return 1;
    if(str.CompareNoCase(L"l")==0)return 1;
    if(str.CompareNoCase(L"t")==0)return 1;
    if(str.CompareNoCase(L"x")==0)return 1;

    if(str.CompareNoCase(L"--")==0)return 1;
    if(str.CompareNoCase(L"-bd")==0)return 1;
    if(    strcmpi_x(str,L"-ao")==0)return 1;
    if(    strcmpi_x(str,L"-o")==0)return 1;
    if(str.CompareNoCase(L"-slt")==0)return 1;
    if(str.CompareNoCase(L"-y")==0)return 1;

    if(str.CompareNoCase(L"-ssc-")==0)return 1;
    if(str.CompareNoCase(L"-ssc")==0)return 1;
    if(str.CompareNoCase(L"-scsutf-8")==0)return 1;
    if(str.CompareNoCase(L"-scswin")==0)return 1;
    if(str.CompareNoCase(L"-scsdos")==0)return 1;

    return 0;
}

/*  #########################################################################  */

CSysString Int64ToString(Int64 a,int pad=0)
{
    CSysString dstr(text("012345678901234567890123456789"));
    const TCHAR*ps=_zt(dstr);
    ConvertInt64ToString(a,const_cast<TCHAR*>(ps));
    CSysString res(ps);
    while(res.Length()<pad)
    {
        res=text(" ")+res;
    }
    return res;
}

/*  #########################################################################  */

CSysString combine_path(const CSysString&path0,const CSysString&path1)
{
    return (path0.Compare(text(""))==0)?path1:((path1.Compare(text(""))==0)?path0:(path0+CSysString(dirDelimiter0)+path1));
}

/*  #########################################################################  */

CSysString clean_path(const CSysString&path)
{
    CSysString path_t(path);
#ifdef _WIN32
    while(path_t.Replace(text("/"),dirDelimiter0));
#endif
    if(path_t[path_t.Length()-1]==dirDelimiter0||path_t[path_t.Length()-1]==dirDelimiter1)
    {
        path_t=path_t.Left(path_t.Length()-1);
    }
    return path_t;
}

/*  #########################################################################  */

int is_path_abs(const CSysString&path)
{
    return path.Find(text(".")+CSysString(dirDelimiter0))>=0||path.Find(text(".")+CSysString(dirDelimiter1))>=0||/*path.Find(text("..")+CSysString(dirDelimiter0))>=0||path.Find(text("..")+CSysString(dirDelimiter1))>=0||*/path.Find(text(":"))>=0||path[0]==dirDelimiter0||path[0]==dirDelimiter1;
}

/*  #########################################################################  */

int file_exists(const CSysString&fname)
{
    if(!DoesNameContainWildCard(a2u(fname)))
    {
        NWindows::NFile::NFind::CEnumerator match(fname);
        NWindows::NFile::NFind::CFileInfo fileInfo;
        if(match.Next(fileInfo))
        {
            return fileInfo.IsDir()?2:1;
        }
    }
    return 0;
}
/*  #########################################################################  */

int compare_ext(const CSysString&fname,const CSysString&ext)
{
    CSysString extp(text(""));
    UInt32 i=fname.Length();
    while(i && fname[i]!='.' && fname[i]!=dirDelimiter0 && fname[i]!=dirDelimiter1)
    {
        i--;
    }
    if(fname[i]=='.')
    {
        extp=fname.Mid(i,fname.Length());
    }
    return CompareFileNames(ext,extp)==0;
}

/*  #########################################################################  */

void log(const CSysString&str,int force=0)
{
    static CSysString*plogdata=0;
    if(plogdata==0)
    {
        plogdata=new CSysString;
    }
    CSysString&logdata=plogdata[0];
    logdata+=str;
    if(logFileName.Compare(text(""))==0)
    {
        return;
    }
    if(logdata.Length()<1024&&force==0)
    {
        return;
    }
    if(logdata.Compare(text(""))==0)
    {
        return;
    }
    int ew=0;
    UInt32 ar;
    UInt64 foffs;
    if(file_exists(logFileName)==0)
    {
        NWindows::NFile::NIO::COutFile fwrite;
        if(fwrite.Open(logFileName,CREATE_NEW))
        {
#ifdef _UNICODE
            fwrite.Write("\xFF\xFE",2,ar);
            if(ar!=2)
            {
                ew=1;
            }
#endif
            fwrite.Close();
        }
        else
        {
            ew=1;
        }
    }
    NWindows::NFile::NIO::COutFile fwrite;
    if(fwrite.Open(logFileName,OPEN_EXISTING))
    {
        fwrite.SeekToEnd(foffs);
#ifdef _WIN32
        logdata.Replace(text("\n"),text("\r\n"));
#endif
        UInt32 tw=logdata.Length()*sizeof(TCHAR);
        fwrite.Write(_zt(logdata),tw,ar);
        if(ar!=tw)
        {
            ew=1;
        }
        logdata=CSysString(&_zt(logdata)[ar/sizeof(TCHAR)]);
        fwrite.Close();
    }
    else
    {
        ew=1;
    }
    if(ew)
    {
        g_StdErr<<text("warning: cannot write to log file (")+logFileName+text(")\n");
        g_StdErr.Flush();
    }
    if(force!=0)
    {
        delete plogdata;
        plogdata=0;
    }
}

/*  #########################################################################  */

void logprint(const CSysString&str,UInt32 p=3)
{
    if(p&1)
    {
        if(p&2)
        {
            g_StdErr<<str;
            g_StdErr.Flush();
        }
        else
        {
            g_StdOut<<str;
            g_StdOut.Flush();
        }
    }
    if(p&2)
    {
        log(str);
    }
}

/*  #########################################################################  */

CSysString GetPathFromSwitch(const CSysString&str)
{
    CSysString res=str;
    if(strcmpi_x(a2u(res),L"@")==0)
    {
        res.Delete(0,1);
    }
    if(strcmpi_x(a2u(res),L"--log")==0)
    {
        res.Delete(0,5);
    }
    if(res[0]=='\"'||res[0]=='\'')
    {
        res.Delete(0,1);
    }
    UInt32 l=res.Length();
    if(l)
    {
        if(res[l-1]=='\"'||res[l-1]=='\'')
        {
            res.Delete(l-1,1);
        }
    }
    else
    {
        res=text("");
    }
    return res;
}

/*  #########################################################################  */

int process_mask(const CSysString&base_path,file_proc fp,void*param=0,int split=1,const CSysString&local_path=text(""),const CSysString&mask=text(""))
{
    int EAX=0;
    if(split)
    {
        //this function should work exactly like 7z command-line parser would, hence all the hassle
        CSysString path_t(clean_path(base_path)),mask_t,local_path_t(text("")),last_element;
        if(path_t[0]=='@') // might mean a directory ?
        {
            path_t=GetPathFromSwitch(path_t);
            if(file_exists(path_t)==1)
            {
                UStringVector resultStrings;
                ReadNamesFromListFile(a2u(path_t),resultStrings,codePage);
                for(int i=0;i<resultStrings.Size();i++)
                {
                    EAX+=process_mask(u2a(resultStrings[i]),fp,param);
                }
            }
            return EAX;
        }
        if(DoesNameContainWildCard(a2u(path_t))) //* instead of a single file
        {
            int i=0,j=path_t.Length();
            for(i=0;i<j;i++)
            {
                if(path_t[i]=='?'||path_t[i]=='*')
                {
                    j=i;
                }
            }
            while(j>0&&path_t[j]!=dirDelimiter0&&path_t[j]!=dirDelimiter1)
            {
                j--;
            }
            if(path_t[j]==dirDelimiter0||path_t[j]==dirDelimiter1)
            {
                mask_t=path_t.Mid(j+1);
                path_t=path_t.Left(j);
            }
            else
            {
                mask_t=path_t;
                path_t=text("");
            }
            if(is_path_abs(path_t)==0)
            {
                local_path_t=path_t;
                path_t=text("");
            }
        }
        else
        {
            mask_t=clean_path(u2a(ExtractFileNameFromPath(a2u(path_t)))); //clean path seems to replace unix / with windows \ and removes the last 
            path_t=clean_path(u2a(ExtractDirPrefixFromPath(a2u(path_t))));
            if(is_path_abs(path_t)==0)
            {
                local_path_t=path_t;
                path_t=text("");
            }
            if(path_t.Compare(text(""))==0)
            {
                if(mask_t.Compare(text("."))==0||mask_t.Compare(text(".."))==0) // likely comparing against curr and parent dir (.) and (..)
                {
                    path_t=mask_t;
                    mask_t=text("*");
                }
            }
        }
//        GetRealPath(local_path_t);
        return process_mask(path_t,fp,param,0,local_path_t,mask_t);
    }
    CSysString path=combine_path(base_path,local_path);
    NWindows::NFile::NFind::CFileInfo fileInfo;
    NWindows::NFile::NFind::CEnumerator match_file(combine_path(path,text("*")));
    while(match_file.Next(fileInfo))
    {
        if(!fileInfo.IsDir())
        {
            if(CompareWildCardWithName(a2u(mask),a2u(combine_path(local_path,fileInfo.Name)))||CompareWildCardWithName(a2u(mask),a2u(fileInfo.Name)))
            {
                EAX+=fp?fp(fileInfo,combine_path(path,fileInfo.Name),combine_path(local_path,fileInfo.Name),param):1;
            }
        }
    }
    NWindows::NFile::NFind::CEnumerator match_dir(combine_path(path,text("*")));
    while(match_dir.Next(fileInfo))
    {
        if(fileInfo.IsDir())
        {
            if(fp)
            {
                if(CompareWildCardWithName(a2u(mask),a2u(combine_path(local_path,fileInfo.Name)))||CompareWildCardWithName(a2u(mask),a2u(fileInfo.Name)))
                {
                    EAX+=fp(fileInfo,combine_path(path,fileInfo.Name),combine_path(local_path,fileInfo.Name),param);
                }
            }
            if(CompareWildCardWithName(a2u(mask),a2u(fileInfo.Name)))
            {
                EAX+=process_mask(base_path,fp,param,split,combine_path(local_path,fileInfo.Name),text("*"));
            }
            else
            {
                EAX+=process_mask(base_path,fp,param,split,combine_path(local_path,fileInfo.Name),mask);
            }
        }
    }
    return EAX;
}

/*  #########################################################################  */

int fenum(const NWindows::NFile::NFind::CFileInfo&fileInfo,const CSysString&fname,const CSysString&local_path,void*x)
{
    finfo*fi=(finfo*)x;
    if(a2u(fileInfo.Name).CompareNoCase(a2u(fi->fileInfo.Name))<0||fi->fileInfo.Name.Compare(text(""))==0)
    {
        fi->fileInfo=fileInfo;
    }
    fi->tcount++;
    if(fileInfo.IsDir())
    {
        if(fi->dirlist)
        {
            fi->dirlist->Add(fname);
        }
        fi->dcount++;
    }
    else
    {
        if(fi->filelist)
        {
            fi->filelist->Add(fname);
        }
        fi->fcount++;
        fi->ttl_fs+=fileInfo.Size;
        if(fileInfo.Size>fi->max_fs)
        {
            fi->max_fs=fileInfo.Size;
        }
        if(fileInfo.Size<fi->min_fs)
        {
            fi->min_fs=fileInfo.Size;
        }
        fi->avg_fs=fi->ttl_fs/fi->fcount;
        if(debug&&fi->debugprint)
        {
            logprint(text("processing ")+Int64ToString(fi->fcount)+text(",")+Int64ToString(fi->ttl_fs)+text(":\t")+local_path+text("\n"),~2);
        }
    }
    return 1;
}

/*  #########################################################################  */

int file_exists(const CSysString&fname,CSysString&fdn)
{
    int EAX=0;
    fdn=text("");
    finfo fi;
    memset(((char*)&fi)+sizeof(fi.fileInfo),0,sizeof(finfo)-sizeof(fi.fileInfo));
    if(process_mask(fname,fenum,&fi))
    {
        fdn=fi.fileInfo.Name;
        EAX=fi.fileInfo.IsDir()?2:1;
    }
    if(fname[0]=='@'||DoesNameContainWildCard(a2u(fname)))
    {
        EAX+=4;
    }
    return EAX; // 1 for files, 2 for dir, +4 if wildcard or has @
}

/*  #########################################################################  */

int is_t7z(const CSysString&fname, char *buffer)
{
    //0 - not a 7z archive
    //1 - t7z archive
    //2 - 7z archive
    int ist7z=0;

    if(file_exists(fname)==1)
    {
        NWindows::NFile::NIO::CInFile fread;
        if(fread.Open(fname))
        {
            UInt32 ar,offs;
            offs=0;
            fread.SeekToBegin();
            fread.Read(buffer+offs,(crcsz+t7zsig_size+4),ar);
            if(ar<(crcsz+t7zsig_size+4))
            {
                if(ar>=t7zsig_size+4)
                {
                    ar-=t7zsig_size+4;
                }
                if(ar<NArchive::N7z::kSignatureSize)
                {
                    ar=NArchive::N7z::kSignatureSize;
                }
                memset(buffer+offs+ar,0,crcsz-ar);
            }
            offs=crcsz;
            UInt64 foffs;
            fread.GetLength(foffs);
            foffs=foffs<(crcsz+t7zsig_size+4)?0:foffs-(crcsz+t7zsig_size+4);
            fread.Seek(foffs,foffs);
            fread.Read(buffer+offs,(crcsz+t7zsig_size+4),ar);
            if(ar<(crcsz+t7zsig_size+4))
            {
                if(ar>=t7zsig_size+4)
                {
                    ar-=t7zsig_size+4;
                }
                if(ar<NArchive::N7z::kSignatureSize)
                {
                    ar=NArchive::N7z::kSignatureSize;
                }
                memcpy(buffer+crcsz*2+t7zsig_size+4+8,buffer+offs+ar,t7zsig_size+4);
                memset(buffer+offs+ar,0,crcsz-ar);
                memcpy(buffer+crcsz*2+8,buffer+crcsz*2+t7zsig_size+4+8,t7zsig_size+4);
            }
            else
            {
                if(ar>=t7zsig_size+4)
                {
                    ar-=t7zsig_size+4;
                }
                if(ar<NArchive::N7z::kSignatureSize)
                {
                    ar=NArchive::N7z::kSignatureSize;
                }
                memcpy(buffer+crcsz*2+t7zsig_size+4+8,buffer+offs+ar,t7zsig_size+4);
                memcpy(buffer+crcsz*2+8,buffer+crcsz*2+t7zsig_size+4+8,t7zsig_size+4);
            }
            fread.GetLength(foffs);
            foffs-=t7zsig_size+4;
            memcpy(buffer+crcsz*2,&foffs,8);
            if(memcmp(buffer,NArchive::N7z::kSignature,NArchive::N7z::kSignatureSize)==0)
            {
                ist7z=2;
                if(compare_t7zsig(buffer+crcsz*2+4+8))
                {
                    UInt32 _crc32=*((UInt32*)(buffer+crcsz*2+8));
                    *((UInt32*)(buffer+crcsz*2+8))=-1;
                    UInt32 crc32=CrcCalc(buffer,crcsz*2+8+t7zsig_size+4);
                    if(crc32==_crc32)
                    {
                        ist7z=1;
                    }
                }
            }
            fread.Close();
        }
        else
        {
            logprint(text("error: cannot read file: ")+fname+text("\n"));
        }
    }
    return ist7z;
}

/*  #########################################################################  */

bool addt7zsig(const CSysString&fname, char * buffer)
{
    if(cmpro_wa_1)
    {
        if(g_createnonsolid[recursion_id])
        {
            return true;
        }
    }
    bool EAX=0;
    if(file_exists(fname)==1)
    {
        NWindows::NFile::NIO::CInFile fread;
        if(fread.OpenShared(fname,true)) // open the 
        {
            UInt32 ar,offs;
            offs=0;
            fread.SeekToBegin();
            fread.Read(buffer+offs,crcsz,ar); //crcsz is const (128)
            if(ar<crcsz)
            {
                memset(buffer+offs+ar,0,crcsz-ar); // if ar < 128, fill 0s from position ar to make it equal to 128
            }
            offs=crcsz;
            UInt64 foffs;
            fread.GetLength(foffs);
            foffs=foffs<crcsz?0:foffs-crcsz;
            fread.Seek(foffs,foffs);
            fread.Read(buffer+offs,crcsz,ar);
            if(ar<crcsz)
            {
                memset(buffer+offs+ar,0,crcsz-ar);
            }
            fread.GetLength(foffs);
            memcpy(buffer+crcsz*2,&foffs,8);
            *((UInt32*)(buffer+crcsz*2+8))=-1;
            memcpy(buffer+crcsz*2+8+4,get_t7zsig(),t7zsig_size);
            UInt32 crc32=CrcCalc(buffer,crcsz*2+8+t7zsig_size+4);
            NWindows::NFile::NIO::COutFile fwrite;
            if(fwrite.Open(fname,OPEN_EXISTING))
            {
                fwrite.SeekToEnd(foffs);
                fwrite.Write(&crc32,4,ar);
                if(ar!=4)
                {
                    logprint(text("error: cannot write file: ")+fname+text("\n"));
                }
                else
                {
                    fwrite.Write(get_t7zsig(),t7zsig_size,ar);
                    if(ar!=t7zsig_size)
                    {
                        logprint(text("error: cannot write file: ")+fname+text("\n"));
                    }
                    else
                    {
                        EAX=1;
                    }
                }
                fwrite.Close();
            }
            else
            {
                logprint(text("error: cannot write file: ")+fname+text("\n"));
            }
            fread.Close();
        }
        else
        {
            logprint(text("error: cannot read file: ")+fname+text("\n"));
        }
    }
    return EAX;
}

/*  #########################################################################  */

bool breakt7zsig(const CSysString&fname)
{
    bool EAX=0;
    NWindows::NFile::NDirectory::MySetFileAttributes(fname,0);
    NWindows::NFile::NIO::COutFile fwrite;
    if(fwrite.Open(fname,OPEN_EXISTING))
    {
        UInt64 foffs;
        fwrite.GetLength(foffs);
        foffs-=1;
        fwrite.Seek(foffs,foffs);
        if(fwrite.SetEndOfFile())
        {
            EAX=1;
        }
        else
        {
            logprint(text("error: cannot write file: ")+fname+text("\n"));
        }
        fwrite.Close();
    }
    else
    {
        logprint(text("error: cannot write file: ")+fname+text("\n"));
    }
    return EAX;
}

/*  #########################################################################  */

CSysString tmp_add_fname(const CSysString&fname,const int ad)
{
    CSysString path=clean_path(u2a(ExtractDirPrefixFromPath(a2u(fname))));
    CSysString name=clean_path(u2a(ExtractFileNameFromPath(a2u(fname))));
    CSysString dirname=combine_path(text(".."),text("t7z_tmpadddir"));
    dirname=combine_path(path,dirname);
    if(ad>0)
    {
        MyCreateDirectory(dirname);
    }
    if(ad<0)
    {
        MyRemoveDirectory(dirname);
    }
    return combine_path(dirname,text("tmp_")+name+text(".t7ztmp"));
}

/*  #########################################################################  */

#ifdef _WIN32
STARTUPINFO snfo;
PROCESS_INFORMATION pi;
int cp(const CSysString&app,const CSysString&cmd)
{
    DWORD eax=-1;
    snfo.cb=sizeof(STARTUPINFO);
    memset(((char*)&snfo)+4,0,snfo.cb-4);
    BOOL PS=CreateProcess(app,_zt(cmd),0,0,FALSE,CREATE_SUSPENDED,0,0,&snfo,&pi);
    if(PS)
    {
        if(cmpro_wa_1)
        {
            if(g_createnonsolid_r[recursion_id])
            {
                DWORD bp=-1,td;
                DWORD ba=((DWORD)GetModuleHandle(0))&0xFFFFF000;
                VirtualProtectEx(pi.hProcess,(void*)ba,16,PAGE_READWRITE,&td);
                WriteProcessMemory(pi.hProcess,(void*)(ba+3),(void*)&bp,1,0);
                g_createnonsolid_r[recursion_id]=0;
            }
        }
        ResumeThread(pi.hThread);
        WaitForSingleObject(pi.hProcess,INFINITE);
        GetExitCodeProcess(pi.hProcess,&eax);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    return *((int*)&eax);
}
#endif

/*  #########################################################################  */

bool execute(const CSysString&app,const CSysString&cmd) //execute a system command (7zip external)
{
#ifdef _WIN32
    int EAX=cp(app,cmd);
#else
    int EAX=_tsystem(text("\"")+cmd+text("\""));
#endif
    if(debug)
    {
        if(EAX!=0)
        {
            logprint(text("command ")+cmd+text(" returned with error code ")+Int64ToString(EAX));
            switch(EAX)
            {
            case 1:
                logprint(text(" : "));
                logprint(text("Warning (Non fatal error(s)). For example, one or more files were locked by some other application, so they were not compressed."));
                break;
            case 2:
                logprint(text(" : "));
                logprint(text("Fatal error"));
                break;
            case 7:
                logprint(text(" : "));
                logprint(text("Command line error"));
                break;
            case 8:
                logprint(text(" : "));
                logprint(text("Not enough memory for operation"));
                break;
            case 255:
                logprint(text(" : "));
                logprint(text("User stopped the process"));
                break;
            case -1:
                {
                    logprint(text(", system ")+Int64ToString(errno));
                    switch(errno)
                    {
                    case E2BIG:
                        logprint(text(" : "));
                        logprint(text("Argument list (which is system dependent) is too big."));
                        break;
                    case ENOENT:
                        logprint(text(" : "));
                        logprint(text("Command interpreter cannot be found."));
                    case ENOEXEC:
                        logprint(text(" : "));
                        logprint(text("Command-interpreter file has invalid format and is not executable."));
                    case ENOMEM:
                        logprint(text(" : "));
                        logprint(text("Not enough memory is available to execute command; or available memory has been corrupted; or invalid block exists, indicating that process making call was not allocated properly."));
                    }
                }
                break;
            }
            logprint(text("\n"));
        }
    }
    return EAX==0;
}

/*  #########################################################################  */

bool DeleteFileAlways(const CSysString&src)
{
    bool EAX=NWindows::NFile::NDirectory::DeleteFileAlways(src);
    if(!EAX)
    {
        logprint(text("error: cannot delete file: ")+src+text("\n"));
    }
    return EAX;
}

/*  #########################################################################  */

bool MyMoveFile(const CSysString&src,const CSysString&dst)
{
    bool EAX=NWindows::NFile::NDirectory::MyMoveFile(src,dst);
    if(!EAX)
    {
        logprint(text("error: cannot move file: ")+src+text(" -> ")+dst+text("\n"));
    }
    return EAX;
}

/*  #########################################################################  */

bool MyCreateDirectory(const CSysString&src)
{
    int fe=file_exists(src);
    if(fe==2)
    {
        return true;
    }
    bool EAX=NWindows::NFile::NDirectory::MyCreateDirectory(src);
    if(!EAX)
    {
        logprint(text("error: cannot create directory: ")+src+text("\n"));
    }
    return EAX;
}

/*  #########################################################################  */

bool MyRemoveDirectory(const CSysString&src)
{
    int fe=file_exists(src);
    if(fe==0)
    {
        return true;
    }
    if(fe==1)
    {
        return false;
    }
    bool EAX=NWindows::NFile::NDirectory::MyRemoveDirectory(src);
    return EAX;
}

/*  #########################################################################  */

bool recompress(const CSysString&fname, CSysString addcmds, char* buffer_inp,bool is7z,bool nonsolid=0)
{
    bool eax=0;
    CSysString current_path;
    NWindows::NFile::NDirectory::MyGetCurrentDirectory(current_path); // current directory
    setenv(text("tmp"),current_path,1);
    NWindows::NFile::NDirectory::CTempDirectory tmpdir;
    if(tmpdir.Create(text("t7z_")))
    {
        //TCHAR*buffer=(TCHAR*)torrent7z::buffer;
        TCHAR*buffer=(TCHAR*)buffer_inp;
		//CSysString*app;
        //Attempting to use recursion for the decompress (
        //bool ext=e7z_exe.Compare(text(""))==0?0:1;

		log(text(""),1);
        
		//Either use the regular 7z or t7z
        /*
		if(ext){ //use external 7zip or not for decompressing to a temp directory
            app=&e7z_exe;
            _stprintf(buffer,text("\"%s\" x -o\"%s\" -ba -y %s-- \"%s\""),_zt(e7z_exe),_zt(tmpdir.GetPath()),_zt(addcmds),_zt(fname)); //ba means silent decryption
        }else{
            app=&t7z_exe;
            _stprintf(buffer,text("\"%s\" x --default-priority --log\"%s\" -o\"%s\" -y %s-- \"%s\""),_zt(t7z_exe),_zt(logFileName),_zt(tmpdir.GetPath()),_zt(addcmds),_zt(fname));
        }
        */
		//try recursive decompression instead of external exe call
        UStringVector decompress_string;
        //UString cmd2 = L"D:\\repos\\torrent7z\\torrent7z\\windows_src\\src\\torrent7z\\o\\t7z.exe x -o " + tmpdir.GetPath() + L" -y "
        //UString cmd2 = L"D:\\repos\\torrent7z\\torrent7z\\windows_src\\src\\torrent7z\\o\\t7z.exe x -o"+ tmpdir.GetPath()+ L"-y "
       // UString cmd2 = L"D:\\repos\\torrent7z\\torrent7z\\windows_src\\src\\torrent7z\\o\\t7z.exe x -o\""+ tmpdir.GetPath()+ L"\" -y "
        UString cmd2 = L"placeholder x -o\""+ tmpdir.GetPath()+ L"\" -y "
                      + addcmds + L" -- " + fname;

        logprint(L"reompress 1");
        logprint(cmd2);
        NCommandLineParser::SplitCommandLine(cmd2,decompress_string);

        recursion_id ++;
        char *buffer2=new char[tmpbufsize];
        int results = t7z_main(decompress_string, buffer2);
        recursion_id --;
        delete [] buffer2;

        if(results == 0)
        {
            log(text(""),1);
            bool clean=0;
            bool proceed=1;
            CSysString newfn;
            UInt32 i=fname.Length();
            while(i && fname[i]!='.' && fname[i]!=dirDelimiter0 && fname[i]!=dirDelimiter1)
            {
                i--;
            }
            if(fname[i]=='.'&&CompareFileNames(a2u(fname.Mid(i,fname.Length())),L".7z")!=0)
            {
                newfn=fname.Left(i)+text(".7z");
                if(file_exists(newfn))
                {
                    logprint(text("error: file \"")+newfn+text("\" already exists, convert operation will be skipped on file \"")+fname+text("\" to prevent data loss\n"));
                    proceed=0;
                }
                clean=1;
            }
            else
            {
                newfn=fname;
            }
            bool cmpro_clean=0;
            if(cmpro_wa_0) //This path would never be exercised, cmpro_wa_0 is set to false
            {
                if(proceed)
                {
                    CSysString new_fname=tmp_add_fname(fname,0);
                    if(file_exists(new_fname))
                    {
                        proceed=0;
                        //_stprintf(buffer,text("\"%s\" x --default-priority --log\"%s\" -o\"%s\" -y %s-- \"%s\""),_zt(t7z_exe),_zt(logFileName),_zt(tmpdir.GetPath()),_zt(addcmds),_zt(new_fname));
                        //if(execute(t7z_exe,buffer))
                        logprint (L"Error: code entered forbidden path");
                        if (false)
                        {
                            finfo fi;
                            memset(((char*)&fi)+sizeof(fi.fileInfo),0,sizeof(finfo)-sizeof(fi.fileInfo));
                            if(process_mask(combine_path(tmpdir.GetPath(),text("*")),fenum,&fi)==0)
                            {
                                eax=DeleteFileAlways(fname)&&DeleteFileAlways(new_fname);
                                tmp_add_fname(fname,-1);
                            }
                            else
                            {
                                proceed=1;
                                cmpro_clean=1;
                            }
                        }
                    }
                }
            }
            else
            {
                if(proceed)
                {
                    proceed=0;
                    finfo fi;
                    memset(((char*)&fi)+sizeof(fi.fileInfo),0,sizeof(finfo)-sizeof(fi.fileInfo));
                    if(process_mask(combine_path(tmpdir.GetPath(),text("*")),fenum,&fi)==0)
                    {
                        eax=DeleteFileAlways(fname);
                    }
                    else
                    {
                        proceed=1;
                    }
                }
            }
            if(proceed)
            {
				//call t7z.exe itself again, with replace-archive and archive name
                //_stprintf(buffer,text("\"%s\" a --default-priority --log\"%s\" --replace-archive -y %s-- \"%s\" \"%s%c*\""),_zt(t7z_exe),_zt(logFileName),_zt(addcmds+(g_stripFileNames[recursion_id]?text("--strip-filenames "):text(""))),_zt(newfn),_zt(tmpdir.GetPath()),dirDelimiter0);

                //cmd2 = L"D:\\repos\\torrent7z\\torrent7z\\windows_src\\src\\torrent7z\\o\\t7z.exe a --replace-archive -y " + addcmds;
                cmd2 = L"placeholder2 a --replace-archive -y " + addcmds;
                if (g_stripFileNames[recursion_id]){
                    cmd2 += L"--strip-filenames ";
                }
                cmd2 += L"-- \"" + newfn + L"\" \"" + tmpdir.GetPath() + dirDelimiter0 ;
                cmd2 += L"*";

                logprint(L"reompress 2");
                logprint(cmd2);
                NCommandLineParser::SplitCommandLine(cmd2,decompress_string);

                recursion_id ++;
                char *buffer2=new char[tmpbufsize];
                int results = t7z_main(decompress_string, buffer2);
                recursion_id --;
                delete [] buffer2;

                if(cmpro_wa_1)
                {
                    if(nonsolid)
                    {
                        g_createnonsolid_r[recursion_id]=1;
                    }
                }
                //if(execute(t7z_exe,buffer))
                if (results == 0)
                {
                    eax=1;
                    if(clean)
                    {
                        eax=DeleteFileAlways(fname);
                    }
                    if(cmpro_wa_0)
                    {
                        if(cmpro_clean)
                        {
                            CSysString new_fname=tmp_add_fname(fname,0);
                            eax=DeleteFileAlways(new_fname);
                            tmp_add_fname(fname,-1);
                        }
                    }
                }
                else
                {
                    logprint(text("error: unable to recompress file: ")+fname+text("\n"));
                }
            }
        }
        else
        {
            logprint(text("error: unable to uncompress file: ")+fname+text("\n"));
        }
        tmpdir.Remove();
    }
    else
    {
        logprint(text("error: unable to create temporary directory\n"));
    }
    return eax;
}

/*  #########################################################################  */

bool convert2nonsolid(const CSysString&fname, CSysString addcmds, char* buffer)
{
    return recompress(fname,addcmds, buffer,1,1);
}

/*  #########################################################################  */

int nodots(const UString&str)
{
    for(int i=/*0*/1;i<str.Length();i++)
    {
        if(str[i]=='.')
        {
            return 0;
        }
    }
    return 1;
}

/*  #########################################################################  */

#ifndef _WIN32
static void GetArguments(int numArguments,const char*arguments[],UStringVector&parts)
{
    parts.Clear();
    for(int i=0;i<numArguments;i++)
    {
        UString s=MultiByteToUnicodeString(arguments[i]);
        parts.Add(s);
    }
}
#endif

/*  #########################################################################  */

//funtion deprecated
/*
void SetCommandStrings(UStringVector&cs) // Seems to store the command string in a static member initially. subsequent calls have the input command string being overwitten with this
{
    static UStringVector*pcs=0;
    if(pcs==0)
    {
        pcs=&cs;
    }
    else
    {
        cs.Clear();
        for(int i=0;i<pcs[0].Size();i++)
        {
            cs.Add(pcs[0][i]);
            if(debug)
            {
                logprint(text("\t")+u2a(cs[i])+text("\n"),~2);
            }
        }
        if(debug)
        {
            logprint(text("\n"),~2);
        }
    }
}
*/
/*  #########################################################################  */

int GetDictionarySize(finfo&fi,bool&solid)
{
    int d=16;
    UInt64 ttl_fs64=fi.ttl_fs; // filesize in bytes
    ttl_fs64+=1024*1024-1; // add 1 mb (prolly for min size)
    ttl_fs64/=1024*1024; //conv to mib
    UInt64 max_fs64=fi.max_fs;
    max_fs64+=1024*1024-1;
    max_fs64/=1024*1024;
    UInt64 min_fs64=fi.min_fs;
    min_fs64+=1024*1024-1;
    min_fs64/=1024*1024;
    UInt64 avg_fs64=fi.avg_fs;
    avg_fs64+=1024*1024-1;
    avg_fs64/=1024*1024;
    int ttl_fs=(int)fix(ttl_fs64,4);
    UInt64 dmaxfs=max_fs64-avg_fs64;// dictionary max and min
    UInt64 dminfs=avg_fs64-min_fs64;
    if((fi.fcount<2)||(dmaxfs>dminfs)/*||(avg_fs64>128)*/) // if only one file or maxdic > mindic
    {
        d=0;
        solid=0;
    }
    else
    {
        d=ttl_fs/2;
        if(max_fs64<60)
        {
            d=min(d,64);
        }
        solid=1;
    }
    //80*11.5+4==924
    d=min(max(16,d),80);  // 16 < d < 80
    return d;
    //note that d cannot be greater than ttl_fs
    //7z will reduce d automatically
}

/*  #########################################################################  */

int convert27z(const CSysString&fname, CSysString addcmds,cfinfo*fi, char* buffer)
{
    int eax=0;
    int ist7z=is_t7z(fname, buffer);
    bool pr=g_forceRecompress[recursion_id]||ist7z!=1;
    if(1)
    {
        const UInt32 tl=10*1000;
        UInt32 t=GetTickCount();
        static UInt32 ot=t+tl;
        static bool prp=0;
        bool prt=((t>ot)&&((fi->fcount+128)<fi->fcountt));
        if(prp&&(pr||prt))
        {
            logprint(text("\n"),~2);
        }
        if((pr&&prp)||prt)
        {
            ot=t+tl;
            logprint(Int64ToString(fi->fcount,7)+text(" out of ")+Int64ToString(fi->fcountt,7)+text(" files processed\n"),~2);
        }
        if(prt)
        {
            prp=0;
        }
        prp|=pr;
    }
    if(pr)
    {
        if(recompress(fname, addcmds,buffer, ist7z!=0))
        {
            fi->fcountp++;
        }
        else
        {
            fi->fcounte++;
        }
        eax=1;
    }
    else
    {
        fi->fcountr++;
    }
    fi->fcount++;
    return eax;
}

/*  #########################################################################  */

void print_usage()
{
    logprint(usage_info,~2);
}

/*  #########################################################################  */

void print_copyright()
{
#ifdef _UNICODE
    CSysString blk(text(""));
    if(blk.Compare(CSysString(_tgetenv(_zt(MultiByteToUnicodeString(t7zsig_str,CP_ACP)))))==0)
    {
        setenv(_zt(MultiByteToUnicodeString(t7zsig_str,CP_ACP)),text("1"),1);
        if(!g_nocopyright[recursion_id])
        {
            logprint(text("\n")+MultiByteToUnicodeString(t7zsig_str,CP_ACP)+text("/")+text(__TIMESTAMP__)+text("\n"),~2);
            logprint(text("using ")+MultiByteToUnicodeString(k7zCopyrightString,CP_ACP)+text("\n\n"),~2);
        }
        g_firstInstance[recursion_id]=1;
    }
#else
    CSysString blk(text(""));
    if(blk.Compare(CSysString(_tgetenv(t7zsig_str)))==0)
    {
        setenv(t7zsig_str,text("1"),1);
        if(!g_nocopyright[recursion_id])
        {
            logprint(text("\n")+CSysString(t7zsig_str)+text("/")+text(__TIMESTAMP__)+text("\n"),~2);
            logprint(text("using ")+CSysString(k7zCopyrightString)+text("\n\n"),~2);
        }
        g_firstInstance[recursion_id]=1;
    }
#endif
}

/*  #########################################################################  */

int t7z_main(UStringVector commandStrings, char *buffer)
{
    CSysString addcmds(text(""));
    for(int i=1;i<commandStrings.Size();i++)  //size seems to be like leng (argv[]) ie. t7z.exe -switch file = size 3
    {
        if(commandStrings[i].CompareNoCase(L"-ba")==0) // disables copyright info
        {
            g_nocopyright[recursion_id]=1;
            commandStrings.Delete(i);
            i--;
        }
    }
    print_copyright(); // displays this file name and 7z copyright
    int operation_mode=1;
    int no_more_switches=0;
    int replace_archive=0;
    for(int i=1;i<commandStrings.Size();i++)
    {
        if(is_command(commandStrings[i])) // is the split string a command letter (e.g. a, d, l, e)
        {
            if(is_allowed(commandStrings[i])) // a (add arhive) not in the list, not sure what is_allowed checks for
            {
                operation_mode=777;
            }
            else
            {
                bool knowncmd=0;
                if(commandStrings[i].CompareNoCase(L"a")==0) //ADD
                {
                    operation_mode=2;
                    knowncmd=1;
                }
                if(cmpro_wa)
                {
                    if(commandStrings[i].CompareNoCase(L"d")==0) // delete
                    {
                        operation_mode=3;
                        knowncmd=1;
                    }
                }
                if(!knowncmd) //if not add or delete (probably should have been in is_allowed, but then the op mode is different.
                {
                    operation_mode=0;
                    logprint(text("error: invalid argument: ")+u2a(commandStrings[i])+text("\n")); // seems like u2a converts from ustring to ascii.
                }
                commandStrings.Delete(i);
                i--;
            }
        }
        else  // argument[i] is not a command switch ( this is for the --switch switches)
        {
            if(no_more_switches==0&&is_switch(commandStrings[i])) //is_switch only checks if argument[i] has a '-' prefix. ie. (-a but isn't a command switch)
            {
                if(commandStrings[i].CompareNoCase(L"--")==0)
                {
                    no_more_switches=1;
                }
                int noprint=0;
                if(commandStrings[i].CompareNoCase(L"--replace-archive")==0||commandStrings[i].CompareNoCase(L"-ra")==0)
                {
                    replace_archive=1;
                    noprint=1;
                }
                if(strcmpi_x(commandStrings[i],L"--log")==0||strcmpi_x(commandStrings[i],L"-l")==0)
                {
                    logFileName=GetPathFromSwitch(u2a(commandStrings[i]));
                    noprint=1;
                }
                if(commandStrings[i].CompareNoCase(L"--force-recompress")==0||commandStrings[i].CompareNoCase(L"-fr")==0)
                {
                    g_forceRecompress[recursion_id]=1;
                    noprint=1;
                }
                if(commandStrings[i].CompareNoCase(L"--batch")==0||commandStrings[i].CompareNoCase(L"-b")==0)
                {
                    g_noninteractive[recursion_id]=1;
                    noprint=1;
                }
                if(commandStrings[i].CompareNoCase(L"--default-priority")==0||commandStrings[i].CompareNoCase(L"-dp")==0)
                {
                    g_defaultPriority[recursion_id]=1;
                    noprint=1;
                }
                if(commandStrings[i].CompareNoCase(L"--strip-filenames")==0||commandStrings[i].CompareNoCase(L"-sf")==0)
                {
                    g_stripFileNames[recursion_id]=1;
                    noprint=1;
                }
                if(cmpro_wa)
                {
                    if(commandStrings[i].CompareNoCase(L"--cmpro")==0||commandStrings[i].CompareNoCase(L"-cm")==0)
                    {
                        g_keepnonsolid[recursion_id]=1;
                        g_noninteractive[recursion_id]=1;
                        noprint=1;
                    }
                }
                if(commandStrings[i].CompareNoCase(L"-ssc-")==0)
                {
                    g_CaseSensitive=0;
                    noprint=1;
                }
                if(commandStrings[i].CompareNoCase(L"-ssc")==0)
                {
                    g_CaseSensitive=1;
                    noprint=1;
                }
                if(commandStrings[i].CompareNoCase(L"-scsutf-8")==0)
                {
                    codePage=CP_UTF8;
                    noprint=1;
                }
                if(commandStrings[i].CompareNoCase(L"-scswin")==0)
                {
                    codePage=CP_ACP;
                    noprint=1;
                }
                if(commandStrings[i].CompareNoCase(L"-scsdos")==0)
                {
                    codePage=CP_OEMCP;
                    noprint=1;
                }
                if(commandStrings[i].CompareNoCase(L"-y")==0)
                {
                    g_yplus[recursion_id]=1;
                    noprint=1;
                }
                if(commandStrings[i].CompareNoCase(L"-n")==0)
                {
                    g_nplus[recursion_id]=1;
                    noprint=1;
                }
                if(commandStrings[i].CompareNoCase(L"-bd")==0)
                {
                    addcmds+=text("-bd");
                    addcmds+=text(" ");
                    noprint=1;
                }
                if(!is_allowed(commandStrings[i]))
                {
                    if(noprint==0)
                    {
                        logprint(text("warning: unsupported switch ignored: ")+u2a(commandStrings[i])+text("\n"),~2);
                    }
                    commandStrings.Delete(i);
                    i--;
                }
            }
        }
    }
#ifdef _WIN32 // Set the process priority (not really needed imo) (based on a the -dp --defaultpriority switch)
    //if(!g_defaultPriority[recursion_id])
    //{
    //    SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
    //}
#endif
    if(g_yplus[recursion_id]!=0&&g_nplus[recursion_id]!=0) // is always-yes/no enabled
    {
        g_yplus[recursion_id]=0;
    }
    if(g_noninteractive[recursion_id])
    {
        if(g_yplus[recursion_id]==0&&g_nplus[recursion_id]==0)
        {
            g_nplus[recursion_id]=1;
        }
    }
    NWindows::NFile::NDirectory::CTempDirectory tmpdir; //temp dir for intermediate shit
    UStringVector t7z_commandStrings;
    //SetCommandStrings(t7z_commandStrings);
	CSysString tmp2 = commandStrings[0];
    t7z_commandStrings.Add(tmp2);//// <= T7 command strings start here
    {
        CSysString cpath;
        NWindows::NFile::NDirectory::MyGetCurrentDirectory(cpath); // current directory
        if(g_firstInstance[recursion_id]) //not sure why it would want to check this. Would this function be called multiple times?
        {
            setenv(text("tmp"),cpath,1);
            if(tmpdir.Create(text("t7z_")))
            {
                setenv(text("tmp"),_zt(tmpdir.GetPath()),1);
            }
            else
            {
                logprint(text("error: unable to create temporary directory\n"));
            }
        }
        TCHAR buffer[64];
        bool nolog=(logFileName.Compare(text(""))==0);
        if(nolog)
        {
            SYSTEMTIME st;
            GetLocalTime(&st);
            _stprintf(buffer,text("torrent7z_%d%02d%02d%02d%02d%02d.log"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond); //log filename is made using current date, time etc
        }
        //t7z_exe=text("t7z");
        #ifndef _WIN32
        //e7z_exe=text("7z"); //might be external 7z
        if(nolog)
        {
            logFileName=combine_path(cpath,CSysString(buffer));
        }
        #else
        if(nolog)
        {
            if(!((!g_noninteractive[recursion_id])&&g_IsParentGui[recursion_id]))
            {
                logFileName=combine_path(cpath,CSysString(buffer));
            }
        }
        //cpath=text("");
        //NWindows::NDLL::MyGetModuleFileName(0,cpath); // gets the current executables full path. Stores inside cpath and... put it in the registry .. ?
        /*
        if(cpath.Length())
        {
            t7z_exe=cpath;
            HKEY hKey;
            DWORD tb;
            if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,text("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\")+u2a(ExtractFileNameFromPath(a2u(cpath))),0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKey,&tb)==0)
            {
                RegSetValueEx(hKey,text(""),0,REG_SZ,(LPBYTE)_zt(cpath),(cpath.Length()+1)*sizeof(TCHAR));
                RegCloseKey(hKey);
            }
        }
        */
        if(nolog)
        {
            if((!g_noninteractive[recursion_id])&&g_IsParentGui[recursion_id])
            {
                cpath=clean_path(u2a(ExtractDirPrefixFromPath(a2u(cpath)))); //gets the current directory (dir path sans exe filename)
                logFileName=combine_path(cpath,CSysString(buffer));
            }
        }
        /*
        e7z_exe=text("7z.exe");
        if(file_exists(e7z_exe)!=1) // is 7z.exe not there ?
        {
            e7z_exe=text("");
            DWORD tb,tp;
            HKEY hKey;
            if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,text("SOFTWARE\\7-Zip"),0,KEY_READ,&hKey)==0) // probably opens the key or path to 7zip? (system wide)
            {
                tb=tmpbufsize;
                if(RegQueryValueEx(hKey,text("Path"),0,&tp,(LPBYTE)&buffer,&tb)==0)
                {
                    e7z_exe=combine_path(buffer,text("7z.exe")); // sneak btrd gets the 7z.exe path from registery
                }
            }
        }
        if(file_exists(e7z_exe)!=1) // check it again after getting the path from the reg (if 7zip was instlled only for user)
        {
            e7z_exe=text("");
            DWORD tb,tp;
            HKEY hKey;
            if(RegOpenKeyEx(HKEY_CURRENT_USER,text("SOFTWARE\\7-Zip"),0,KEY_READ,&hKey)==0)
            {
                tb=tmpbufsize;
                if(RegQueryValueEx(hKey,text("Path"),0,&tp,(LPBYTE)&buffer,&tb)==0)
                {
                    e7z_exe=combine_path(buffer,text("7z.exe"));
                }
            }
        }
        if(file_exists(e7z_exe)!=1) //if still not found, put a blank string to flag the absence
        {
            e7z_exe=text("");
        }
        */
#endif
    }
    if(commandStrings.Size()<2) // probably means only one argument was found i.e. sysargv len < 2
    {
        print_usage();
        return NExitCode::kUserError;
    }
    if(operation_mode==0) // operation_mode was set earlier via the switch 2 means add archive
    {
        print_usage();
        return NExitCode::kUserError;
    }
    if(operation_mode==1)  // 1 is the default value (prob set when no command string is there
    {
        for(int i=1;i<commandStrings.Size();i++)
        {
            if(is_command(commandStrings[i]))
            {
                commandStrings.Delete(i);
                i--;
            }
            else
            {
                if(no_more_switches==0&&is_switch(commandStrings[i]))
                {
                    if(commandStrings[i].CompareNoCase(L"--")==0)
                    {
                        no_more_switches=1;
                    }
                    commandStrings.Delete(i);
                    i--;
                }
            }
        }
        finfo pi;
        memset(((char*)&pi)+sizeof(pi.fileInfo),0,sizeof(finfo)-sizeof(pi.fileInfo));
        CSysStringVector filelist;
        pi.filelist=&filelist;
        for(int i=1;i<commandStrings.Size();i++)
        {
            int cln=0;
            CSysString fn;
            if(file_exists(u2a(commandStrings[i]))==2)
            {
                fn=combine_path(u2a(commandStrings[i]),text("*"));
                cln=1;
            }
            else
            {
                fn=u2a(commandStrings[i]);
            }
            int fls=filelist.Size();
            process_mask(fn,fenum,&pi);
            if(cln)
            {
                for(int j=fls;j<filelist.Size();j++)
                {
                    int pass=0;
                    for(int k=0;knownext[k].ext;k++)
                    {
                        if(compare_ext(filelist[j],knownext[k].ext))
                        {
                            if(knownext[k].a==0)
                            {
                                if(g_yplus[recursion_id])
                                {
                                    knownext[k].a=1;
                                }
                                if(g_nplus[recursion_id])
                                {
                                    knownext[k].a=2;
                                }
                            }
                            pass=knownext[k].a==1;
                            if(knownext[k].a==0)
                            {
                                int rep=0;
                                while(rep==0)
                                {
                                    logprint(text("Convert \"")+filelist[j]+text("\" to t7z?\n Yes (y)/No (n)/Yes to All *")+knownext[k].ext+text(" (y+,a)/No to All *")+knownext[k].ext+text(" (n+,-)/Quit (q)?  "),~2);
                                    TCHAR reply_buff[64];
                                    reply_buff[0]=16;
                                    TCHAR*reply=_getts(reply_buff);
                                    logprint(text("\n"),~2);
                                    if(reply==0)
                                    {
                                    }
                                    else
                                    {
                                        if(lstrcmpi(reply,text("y"))==0||lstrcmpi(reply,text("yes"))==0)
                                        {
                                            pass=1;
                                            rep=1;
                                        }
                                        if(lstrcmpi(reply,text("n"))==0||lstrcmpi(reply,text("no"))==0)
                                        {
                                            rep=1;
                                        }
                                        if(lstrcmpi(reply,text("y+"))==0||lstrcmpi(reply,text("a"))==0||lstrcmpi(reply,text("yes to all"))==0)
                                        {
                                            pass=1;
                                            knownext[k].a=1;
                                            rep=1;
                                        }
                                        if(reply[0]==0||lstrcmpi(reply,text("n+"))==0||lstrcmpi(reply,text("-"))==0||lstrcmpi(reply,text("no to all"))==0)
                                        {
                                            knownext[k].a=2;
                                            rep=1;
                                        }
                                        if(lstrcmpi(reply,text("q"))==0||lstrcmpi(reply,text("quit"))==0)
                                        {
                                            g_noninteractive[recursion_id]=1;
                                            return 0;
                                        }
                                    }
                                }
                            }
                            break;
                        }
                    }
                    if(!pass)
                    {
                        filelist.Delete(j);
                        pi.fcount--;
                        j--;
                    }
                }
            }
        }
        cfinfo fi;
        memset(&fi,0,sizeof(fi));
        fi.fcountt=pi.fcount;
        for(int i=0;i<filelist.Size();i++)
        {
            convert27z(filelist[i], addcmds ,&fi , buffer);
        }
        if(fi.fcount==0)
        {
            logprint(text("\nerror: no file(s) to process\n"));
        }
        else
        {
            int print=fi.fcounte?~0:~2;
            logprint(text("\n")+Int64ToString(fi.fcount)+text(" file(s) processed, ")+Int64ToString(fi.fcountp)+text(" file(s) converted\n"),print);
            if(fi.fcountr)
            {
                logprint(Int64ToString(fi.fcountr)+text(" file(s) were already in t7z format\n"),print);
            }
            logprint(text("\n"),print);
            if(fi.fcounte)
            {
                logprint(text("there were ")+Int64ToString(fi.fcounte)+text(" error(s)\n"),print);
                logprint(text("check error log for details\n\n"),~2);
            }
        }
    }
    if(operation_mode==2)
    {
		//useless construct to analyze the array in debugger
		CSysString tmp2;
        UString*archive,*pd,*sd;
        t7z_commandStrings.Add(L"a");
        t7z_commandStrings.Add(L"-r");  //recursive
        t7z_commandStrings.Add(L"-t7z");  //format 7z
        t7z_commandStrings.Add(L"-mx=9");  //compression max
        t7z_commandStrings.Add(L"");
        sd=&t7z_commandStrings[t7z_commandStrings.Size()-1];
        t7z_commandStrings.Add(L"-mf=on"); //compression filter, not sure what =on means, doc suggest mf
        t7z_commandStrings.Add(L"-mhc=on"); //header compression
        t7z_commandStrings.Add(L"-mhe=off");  //header encryption
        t7z_commandStrings.Add(L"-mmt=2");     //multithread
        t7z_commandStrings.Add(L"-mtc=off"); // creation times, access and mod times disable
        t7z_commandStrings.Add(L"-mta=off");
        t7z_commandStrings.Add(L"-mtm=off");
        t7z_commandStrings.Add(L"");
        pd=&t7z_commandStrings[t7z_commandStrings.Size()-1];
        t7z_commandStrings.Add(L"-ba");  // supress headers (not a t7z (v4.65) command
        no_more_switches=0;
        for(int i=1;i<commandStrings.Size();i++) // interstingly starts after the first argv[]
        { 
			tmp2 = commandStrings[i];
            if(is_command(commandStrings[i])) //is_command checks for actual command letter like a
            {
                commandStrings.Delete(i);
                i--;
            }
            else
            {
                if(no_more_switches==0&&is_switch(commandStrings[i]))  //isswitch just checks for presence of -
                {
                    if(commandStrings[i].CompareNoCase(L"--")==0)
                    {
                        no_more_switches=1;
                    }
                    else
                    {
                        t7z_commandStrings.Add(commandStrings[i]);
                    }
                    commandStrings.Delete(i);
                    i--;
                }
            }
        }
        t7z_commandStrings.Add(L"--");
        t7z_commandStrings.Add(L"");
        archive=&t7z_commandStrings[t7z_commandStrings.Size()-1];
		
		
		//for (int j=0; j <2; j++){ tmp2 = archive[j];}
		finfo fi; //struct contains windows file paths, counts etc see line 80
        memset(((char*)&fi)+sizeof(fi.fileInfo),0,sizeof(finfo)-sizeof(fi.fileInfo));
        fi.debugprint=1;
        bool fnotfound=0;
        int fe=0;
        for(int i=1;i<commandStrings.Size();i++)
        {
			tmp2 = commandStrings[i];
            if(fe==0)
            {
                if(replace_archive)
                {
                    archive[0]=commandStrings[i];
                    fe=1;
                    continue;
                }
                else
                {
                    CSysString fn;
                    fe=file_exists(u2a(commandStrings[i]),fn); // it returns 0, 1, 4 etc (multiple stuff) 1 for file, 2 dir, +4 for wildcard
                    if(fe==0)
                    {
                        archive[0]=commandStrings[i];
                    }
                    else
                    {
                        if(fe!=4) // not a wildcard, I think ?
                        {
                            if(fe&6)
                            {
                                archive[0]=a2u(fn);
                            }
                            else
                            {
                                archive[0]=a2u(clean_path(combine_path(u2a(ExtractDirPrefixFromPath(commandStrings[i])),fn)));
                            }
                            if(((fe&2)!=0)||(is_t7z(u2a(archive[0]), buffer)==0)) // if not dir or not a t7z (guess this is where they add the archive name if not specified)
                            {
                                archive[0]+=L".7z";
                                t7z_commandStrings.Add(commandStrings[i]);
                                if(process_mask(u2a(commandStrings[i]),fenum,&fi)==0)
                                {
                                    fnotfound=1;
                                    logprint(text("error: no files match: ")+u2a(commandStrings[i])+text("\n"));
                                }
                            }
                        }
                    }
                    if(fe==4)
                    {
                        fe=0; // allows the next loop
                    }
                    else
                    {
                        fe=1; // disallows the next loop
                        continue;
                    }
                }
            }
            t7z_commandStrings.Add(commandStrings[i]);
            if(process_mask(u2a(commandStrings[i]),fenum,&fi)==0)
            {
                fnotfound=1;
                logprint(text("error: no files match: ")+u2a(commandStrings[i])+text("\n"));
            }
        }
        if(fi.fcount==0||fnotfound)
        {
            if(fi.fcount==0)
            {
                logprint(text("error: no files to process\n"));
            }
            return NExitCode::kUserError;
        }
        bool solid;
        int ds=GetDictionarySize(fi,solid); // looks at avg, max, min file sized and returns a dictionary size and if it should be solid or not
        if(solid)
        {
            sd[0]=UString(L"-ms=")+a2u(Int64ToString(fix(fi.fcount+128,1024)))+UString(L"f4g");
            if(debug)
            {
                logprint(text("\t***\tsolid mode activated\n"),~2);
            }
        }
        else
        {
            //sd[0]=UString(L"-ms=off");//disabled for now
            sd[0]=UString(L"-ms=")+a2u(Int64ToString(fix(fi.fcount+128,1024)))+UString(L"f4g"); //ms is block size, dunno what f4g is (e.g. -ms=1024f4g)
            if(debug)
            {
                logprint(text("\t***\tno use for solid mode\n"),~2);
            }
        }
        pd[0]=UString(L"-m0=LZMA:a1:d")+a2u(Int64ToString(ds))+UString(L"m:mf=BT4:fb128:mc80:lc4:lp0:pb2"); // specifies what compression algo for whih streams.
        if(archive[0][0]==0)
        {
            archive[0]=L"default";
        }
        if(nodots(archive[0]))  //no dots in string (if archive name was inputted)
        {
            archive[0]+=L".7z";
        }
        if(!replace_archive)
        {
            fe=file_exists(u2a(archive[0])); // check if the output file exists
            if(fe!=0)
            {
                bool adderr=1;
                if(cmpro_wa)
                {
                    CSysString fname=u2a(archive[0]);
                    int ist7z=is_t7z(fname , buffer);
                    if(ist7z==1)
                    {
                        if(cmpro_wa_0)
                        {
                            if(breakt7zsig(fname))
                            {
                                ist7z=2;
                            }
                        }
                        if(cmpro_wa_1)
                        {
                            if(convert2nonsolid(fname, addcmds,buffer))
                            {
                                ist7z=2;
                            }
                        }
                        if(ist7z!=2)
                        {
                            logprint(text("error: cannot update archive \"")+u2a(archive[0])+text("\"\n"));
                            return NExitCode::kUserError;
                        }
                    }
                    if(ist7z==2)
                    {
                        adderr=0;
                        if(cmpro_wa_0)
                        {
                            archive[0]=a2u(tmp_add_fname(fname,1));
                        }
                        t7z_commandStrings.Delete(6,7);
                        t7z_commandStrings.Delete(4,1);
                        sd[0]=L"-mx=1";   //comprexxion level 1.. I think
                        pd[0]=L"-ms=off"; //solid off
                        int eax=main3(
                        //#ifndef _WIN32
                        //argc,newargs
                        //#endif
                        t7z_commandStrings
                        );
                        if(!g_keepnonsolid[recursion_id])
                        {
                            if(!recompress(u2a(archive[0]), addcmds,buffer,1))
                            {
                                eax=NExitCode::kFatalError;
                            }
                        }
                        return eax;
                    }
                }
                if(adderr)
                {
                    logprint(text("error: file \"")+u2a(archive[0])+text("\" already exists, update operation is not supported, aborting\n"));
                    return NExitCode::kUserError;
                }
            }
        }
        UString rarchive=archive[0]; //archive[0] contained the output 7z archive name (ouput_archive.7z.tmp)
        archive[0]+=UString(L".tmp");
        fe=file_exists(u2a(archive[0]));
        if(fe!=0)
        {
            logprint(text("error: file \"")+u2a(archive[0])+text("\" already exists(manually delete .tmp file, probably a leftover)\n"));
            return NExitCode::kUserError;
        }
        if(fi.tcount==1) // is there only one file
        {
            g_singleFile[recursion_id]=1;
        }
        if(!g_singleFile[recursion_id]) // disable strip filenames if multiple filenames
        {
            g_stripFileNames[recursion_id]=0;
        }
        if(cmpro_wa_1)
        {
            if(g_createnonsolid[recursion_id])
            {
                t7z_commandStrings.Delete(6,7);
                t7z_commandStrings.Delete(4,1);
                sd[0]=L"-mx=1";  ///compression levle mx means no compress I think
                pd[0]=L"-ms=off"; // solid archive
            }
        }//The main3 function actually calls the 7zip UI console main program (t7z-master\src\cpp\7zip\UI\Console\MainAr.cpp)

        int eax=main3(
        //#ifndef _WIN32
        //argc,newargs
        //#endif
        t7z_commandStrings
        );
        if(eax==0)
        {
            eax=NExitCode::kFatalError;
            if(addt7zsig(u2a(archive[0]) , buffer ))
            {
                bool fne=1;
                if(replace_archive&&file_exists(u2a(rarchive)))
                {
                    fne=DeleteFileAlways(u2a(rarchive));
                }
                if(fne)
                {
                    if(g_stripFileNames[recursion_id]&&g_singleFile[recursion_id])
                    {
                        //or else file extension most likely will be lost
                        rarchive=a2u(clean_path(combine_path(u2a(ExtractDirPrefixFromPath(rarchive)),fi.fileInfo.Name)+CSysString(text(".7z"))));
                    }
                    if(MyMoveFile(u2a(archive[0]),u2a(rarchive)))
                    {
                        eax=0;
                    }
                }
            }
        }
        else
        {
            if(file_exists(u2a(archive[0])))
            {
                DeleteFileAlways(u2a(archive[0]));
            }
        }
        return eax;
    }
    if(operation_mode==3)
    {
        if(cmpro_wa)
        {
            if(cmpro_wa_0)
            {
                t7z_commandStrings.Add(L"u");
                t7z_commandStrings.Add(L"-up1q3r3x3y3z3w3");
            }
            if(cmpro_wa_1)
            {
                t7z_commandStrings.Add(L"d");
            }
            t7z_commandStrings.Add(L"-ba");
            t7z_commandStrings.Add(L"-mx=1");
            t7z_commandStrings.Add(L"-ms=off");
            no_more_switches=0;
            for(int i=1;i<commandStrings.Size();i++)
            {
                if(is_command(commandStrings[i]))
                {
                    commandStrings.Delete(i);
                    i--;
                }
                else
                {
                    if(no_more_switches==0&&is_switch(commandStrings[i]))
                    {
                        if(commandStrings[i].CompareNoCase(L"--")==0)
                        {
                            no_more_switches=1;
                        }
                        commandStrings.Delete(i);
                        i--;
                    }
                }
            }
            t7z_commandStrings.Add(L"--");
            t7z_commandStrings.Add(L"");
            UString*archive=&t7z_commandStrings[t7z_commandStrings.Size()-1];
            int fe=0;
            for(int i=1;i<commandStrings.Size();i++)
            {
                if(fe==0)
                {
                    archive[0]=commandStrings[i];
                    fe=1;
                    continue;
                }
                if(cmpro_wa_0)
                {
                    if(DoesNameContainWildCard(commandStrings[i]))
                    {
                        logprint(text("error: wilcards for delete operation are not implemented yet\n"));
                        return NExitCode::kUserError;
                    }
                }
                t7z_commandStrings.Add(commandStrings[i]);
            }
            if(archive[0][0]==0)
            {
                archive[0]=L"default";
            }
            if(nodots(archive[0]))
            {
                archive[0]+=L".7z";
            }
            fe=file_exists(u2a(archive[0]));
            if(fe)
            {
                CSysString fname=u2a(archive[0]);
                int ist7z=is_t7z(fname, buffer);
                if(ist7z==1)
                {
                    if(cmpro_wa_0)
                    {
                        if(breakt7zsig(fname))
                        {
                            ist7z=2;
                        }
                    }
                    if(cmpro_wa_1)
                    {
                        if(convert2nonsolid(fname, addcmds,buffer))
                        {
                            ist7z=2;
                        }
                    }
                }
                if(ist7z==2)
                {
                    if(cmpro_wa_0)
                    {
                        archive[0]=a2u(tmp_add_fname(fname,1));
                    }
                }
                else
                {
                    fe=0;
                }
            }
            if(fe==0)
            {
                logprint(text("error: cannot update archive \"")+u2a(archive[0])+text("\"\n"));
                return NExitCode::kUserError;
            }
            if(cmpro_wa_0)
            {
                g_isDeleteOp[recursion_id]=1;
            }
            int eax=main3(
             //#ifndef _WIN32
            //argc,newargs
            //#endif
            t7z_commandStrings
            );
            g_isDeleteOp[recursion_id]=0;
            if(!g_keepnonsolid[recursion_id])
            {
                if(!recompress(u2a(archive[0]),addcmds,buffer,1))
                {
                    eax=NExitCode::kFatalError;
                }
            }
            return eax;
        }
    }
    if(operation_mode==777)
    {
        t7z_commandStrings.Add(L"-ba");
        t7z_commandStrings.Add(L"-r");
        for(int i=1;i<commandStrings.Size();i++)
        {
            t7z_commandStrings.Add(commandStrings[i]);
        }
        int eax=main3(
         //#ifndef _WIN32
        //argc,newargs
        //#endif
        t7z_commandStrings
        );
        return eax;
    }
    return NExitCode::kUserError;
}

/*  #########################################################################  */

#ifdef _WIN32
#if !defined(_UNICODE) || !defined(_WIN64)
static inline bool IsItWindowsNT()
{
    OSVERSIONINFO versionInfo;
    versionInfo.dwOSVersionInfoSize=sizeof(versionInfo);
    if(!GetVersionEx(&versionInfo))
    {
        return false;
    }
    return (versionInfo.dwPlatformId==VER_PLATFORM_WIN32_NT);
}
#endif
#endif

/*  #########################################################################  */

}
using namespace torrent7z;

/*  #########################################################################  */

int MY_CDECL main
(
#ifndef _WIN32
    int numArguments,const char*arguments[]
#endif
)
{
    char*buffer;
    initialize_global_switches();
#ifdef _UNICODE
#ifndef _WIN64
    if(!IsItWindowsNT())
    {
        g_StdErr<<text("This program requires Windows NT/2000/2003/2008/XP/Vista\n");
        g_StdErr.Flush();
        return NExitCode::kFatalError;
    }
#endif
#endif
#ifdef _WIN32
    if(!debug)
    {
        SetErrorMode(SEM_FAILCRITICALERRORS);
    }
    SetLastError(0);
    //g_createnonsolid=(((BYTE*)((((DWORD)GetModuleHandle(0))&0xFFFFF000)+3))[0]&0xff)==0xff;
#endif
    //g_stripFileNames=0;
    //g_singleFile=0;
    //g_forceRecompress=0;
    //g_noninteractive=(_isatty(_fileno(stdout))==0||_isatty(_fileno(stderr))==0);
#ifdef _WIN32
    //STARTUPINFO startupInfo;
    //GetStartupInfo(&startupInfo);
    //if((startupInfo.dwFlags&STARTF_USESHOWWINDOW)&&(startupInfo.wShowWindow==0))
    //{
    //    g_noninteractive=1;
    //}
#endif
    codePage=CP_UTF8;
    logFileName=text("");
    buffer=new char[tmpbufsize];
    if(buffer==0)
    {
        logprint(text("error: Can't allocate required memory!\n"));
        return NExitCode::kFatalError;
    }
    //g_IsParentGui=IsParentGui()!=0;
    
    ///g_IsParentGui = false;
    ///g_firstInstance=0;
    ///g_defaultPriority=0;
    ///g_isDeleteOp=0;
    ///g_keepnonsolid=0;
    ///g_yplus=0;
    ///g_nplus=0;
    ///g_nocopyright=0;
    //logprint(text("executing command line: ")+CSysString(GetCommandLine())+text("\n"),~1);
    //log execution started:
    UStringVector commandStrings;
    #ifdef _WIN32
    UString cmdl(GetCommandLineW());
    NCommandLineParser::SplitCommandLine(cmdl,commandStrings);
    #else
    //    GetArguments(numArguments,arguments,commandStrings);
    extern void mySplitCommandLine(int numArguments,const char *arguments[],UStringVector &parts);
    mySplitCommandLine(numArguments,arguments,commandStrings);
    #endif
    int EAX=t7z_main(commandStrings , buffer );
    //log time taken
    log(text(""),1);
#ifdef _WIN32
    if((!g_noninteractive[0])&&g_IsParentGui[0])
    {
        logprint(text("\nPress any key to continue . . . "),~2);
        _gettch();
        logprint(text("\n"),~2);
    }
#endif
    if(buffer)
    {
        delete[] buffer;
    }
    return EAX;
}

/*  #########################################################################  */
