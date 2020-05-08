PROG = t7z.exe
LIBS = $(LIBS) kernel32.lib user32.lib oleaut32.lib advapi32.lib

CFLAGS = $(CFLAGS) -I ../cpp/ \
  -D_NO_CRYPTO \
  -DWIN_LONG_PATH \
  -DCOMPRESS_MT \
  -DCOMPRESS_MF_MT \
  -DBREAK_HANDLER \
  -DUNICODE \
  -D_UNICODE \
  -D_CRT_SECURE_NO_WARNINGS \
  -D_CRT_SECURE_NO_DEPRECATE \
  -D_CRT_NON_CONFORMING_SWPRINTFS \


#-DEXTERNAL_CODECS

#$DEBUG = "TRUE"

#!if "$(DEBUG)"=="TRUE"
#CFLAGS = $(CFLAGS) -DDEBUG /ZI
#LFLAGS = $(LFLAGS) /DEBUG
#!else
LFLAGS = $(LFLAGS) /RELEASE
#!endif

CONSOLE_OBJS = \
  $O\ConsoleClose.obj \
  $O\ExtractCallbackConsole.obj \
  $O\List.obj \
  $O\Main.obj \
  $O\MainAr.obj \
  $O\OpenCallbackConsole.obj \
  $O\PercentPrinter.obj \
  $O\UpdateCallbackConsole.obj \
  $O\UserInputUtils.obj \

COMMON_OBJS = \
  $O\CommandLineParser.obj \
  $O\CRC.obj \
  $O\IntToString.obj \
  $O\ListFileUtils.obj \
  $O\NewHandler.obj \
  $O\StdInStream.obj \
  $O\StdOutStream.obj \
  $O\MyString.obj \
  $O\StringConvert.obj \
  $O\StringToInt.obj \
  $O\UTFConvert.obj \
  $O\MyVector.obj \
  $O\Wildcard.obj \

WIN_OBJS = \
  $O\DLL.obj \
  $O\Error.obj \
  $O\FileDir.obj \
  $O\FileFind.obj \
  $O\FileIO.obj \
  $O\FileName.obj \
  $O\MemoryLock.obj \
  $O\PropVariant.obj \
  $O\PropVariantConversions.obj \
#  $O\Registry.obj \
  $O\Synchronization.obj \
  $O\System.obj \
  $O\Time.obj \

7ZIP_COMMON_OBJS = \
  $O\CreateCoder.obj \
  $O\FilePathAutoRename.obj \
  $O\FileStreams.obj \
  $O\InBuffer.obj \
  $O\InOutTempBuffer.obj \
  $O\FilterCoder.obj \
  $O\LimitedStreams.obj \
  $O\LockedStream.obj \
  $O\MethodId.obj \
  $O\MethodProps.obj \
  $O\OffsetStream.obj \
  $O\OutBuffer.obj \
  $O\ProgressUtils.obj \
  $O\StreamBinder.obj \
  $O\StreamObjects.obj \
  $O\StreamUtils.obj \
  $O\VirtThread.obj \

UI_COMMON_OBJS = \
  $O\ArchiveCommandLine.obj \
  $O\ArchiveExtractCallback.obj \
  $O\ArchiveOpenCallback.obj \
  $O\DefaultName.obj \
  $O\EnumDirItems.obj \
  $O\Extract.obj \
  $O\ExtractingFilePath.obj \
  $O\LoadCodecs.obj \
  $O\OpenArchive.obj \
  $O\PropIDUtils.obj \
  $O\SetProperties.obj \
  $O\SortUtils.obj \
  $O\TempFiles.obj \
  $O\Update.obj \
  $O\UpdateAction.obj \
  $O\UpdateCallback.obj \
  $O\UpdatePair.obj \
  $O\UpdateProduce.obj \
  $O\WorkDir.obj \

AR_COMMON_OBJS = \
  $O\CoderMixer2.obj \
  $O\CoderMixer2MT.obj \
  $O\CrossThreadProgress.obj \
  $O\DummyOutStream.obj \
  $O\HandlerOut.obj \
  $O\InStreamWithCRC.obj \
  $O\ItemNameUtils.obj \
  $O\MultiStream.obj \
  $O\OutStreamWithCRC.obj \
  $O\ParseProperties.obj \


7Z_OBJS = \
  $O\7zCompressionMode.obj \
  $O\7zDecode.obj \
  $O\7zEncode.obj \
  $O\7zExtract.obj \
  $O\7zFolderInStream.obj \
  $O\7zFolderOutStream.obj \
  $O\7zHandler.obj \
  $O\7zHandlerOut.obj \
  $O\7zHeader.obj \
  $O\7zIn.obj \
  $O\7zOut.obj \
  $O\7zProperties.obj \
  $O\7zRegister.obj \
  $O\7zSpecStream.obj \
  $O\7zUpdate.obj \

LZM_OBJS = \
  $O\LzmaArcRegister.obj \
  $O\LzmaFiltersDecode.obj \
  $O\LzmaHandler.obj \
  $O\LzmaIn.obj \

SPLIT_OBJS = \
  $O\SplitHandler.obj \
  $O\SplitHandlerOut.obj \
  $O\SplitRegister.obj \

COMPRESS_OBJS = \
  $O\Bcj2Coder.obj \
  $O\Bcj2Register.obj \
  $O\BcjCoder.obj \
  $O\BcjRegister.obj \
  $O\BranchCoder.obj \
  $O\BranchMisc.obj \
  $O\BranchRegister.obj \
  $O\ByteSwap.obj \
  $O\ByteSwapRegister.obj \
  $O\CopyCoder.obj \
  $O\CopyRegister.obj \
  $O\LzmaDecoder.obj \
  $O\LzmaEncoder.obj \
  $O\LzmaRegister.obj \

C_OBJS = \
  $O\7zCrc.obj \
  $O\Bra.obj \
  $O\Bra86.obj \
  $O\BraIA64.obj \
  $O\Alloc.obj \
  $O\LzmaDec.obj \
  $O\LzmaEnc.obj \
  $O\LzFind.obj \
  $O\LzFindMt.obj \
  $O\Threads.obj \

OBJS = \
  $O\StdAfx.obj \
  $(CONSOLE_OBJS) \
  $(COMMON_OBJS) \
  $(WIN_OBJS) \
  $(7ZIP_COMMON_OBJS) \
  $(UI_COMMON_OBJS) \
  $(AR_COMMON_OBJS) \
  $(7Z_OBJS) \
  $(LZM_OBJS) \
  $(SPLIT_OBJS) \
  $(COMPRESS_OBJS) \
  $(C_OBJS) \
  $(CRC_OBJS) \
  $(T7Z_OBJS) \
  $O\resource.res

T7Z_OBJS = \
  $O\t7z.obj \
  $O\torrent7z_signature.obj \
  $O\w32t.obj \
  $O\usage.obj

!include "../cpp/Build.mak"

$(CONSOLE_OBJS): ../cpp/7zip/UI/Console/$(*B).cpp
	$(COMPL)

$(COMMON_OBJS): ../cpp/Common/$(*B).cpp
	$(COMPL)
$(WIN_OBJS): ../cpp/Windows/$(*B).cpp
	$(COMPL)
$(7ZIP_COMMON_OBJS): ../cpp/7zip/Common/$(*B).cpp
	$(COMPL)
$(UI_COMMON_OBJS): ../cpp/7zip/UI/Common/$(*B).cpp
	$(COMPL)
$(AR_COMMON_OBJS): ../cpp/7zip/Archive/Common/$(*B).cpp
	$(COMPL)

$(7Z_OBJS): ../cpp/7zip/Archive/7z/$(*B).cpp
	$(COMPL)
$(LZM_OBJS): ../cpp/7zip/Archive/Lzma/$(*B).cpp
	$(COMPL)
$(SPLIT_OBJS): ../cpp/7zip/Archive/Split/$(*B).cpp
	$(COMPL)
$(COMPRESS_OBJS): ../cpp/7zip/Compress/$(*B).cpp
	$(COMPL_O2)
$(C_OBJS): ../c/$(*B).c
	$(COMPL_O2)

$(T7Z_OBJS): $(*B).cpp
	$(COMPL)
