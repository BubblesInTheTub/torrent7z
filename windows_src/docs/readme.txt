you can use t7z in two ways:
convert *.7z archives to t7z format - pass directory containing archives
as a first parameter to application(exactly the same as with torrentzip)
to process current directory, pass dot "." as a first parameter

use as compressor - put t7z instead of regular 7z in the settings dialog of the
program you use(ie clrmamepro, goodmerge etc.)


* converting archives:

t7z <file/dir/mask/@listfile>,<file/dir/mask/@listfile>...
if <file> is a 7z archive, recompresses it(if needed) with constant settings.

t7z <dir> processes only *.7z archives
t7z <mask> will convert all files which match <mask> and can be "unpacked" to t7z format
never pass * as a mask, 7z actually can unpack many file formats, not necessarily archives
for example, it can open *.exe files as archives, splitting them by sections
for this to work for archives other than *.7z, you will need a regular 7z installed
example:
t7z *.rar will convert rar archives if external uncompressor can be found


* creating archives:

t7z a <archive> <file/dir/mask/@listfile>,<file/dir/mask/@listfile>...
compresses <file/dir/mask/@listfile> to <archive>
if <archive> exists, produces error and stops
if <archive> cannot be created(non-existent path or invalid filename), produces error and stops
if <file> is actually a <directory>, adds all files/directories in it(but not the directory itself) to archive
if <archive> is omitted, defaults to <first filename>.7z

note that archive name must contain a dot(.), otherwise default extension(.7z) will be appended
dot cannot be the first symbol of the filename
that's not a problem by itself, but you can get unexpected results(ie
if you pass "dummy" as archive name, "dummy.7z" will be created)

note that command "t7z a file.zip file.txt" will create archive file.txt.7z if both files
exist, containing two files file.zip and file.txt, since file.zip most likely is not a 7z
archive, so it's treated as a filename to add to archive, and not as archive name
if it's actually a 7z archive, error will be generated instead


* other options:

--default-priority
 use priority of the calling process for operation(windows default).
 by default, idle priority is used.

--log"logfile.log"
 log messages to logfile
 if omitted random logfile name will be created

--batch
 do not pause at the end of operation, if the parent process is a gui application
 t7z will not pause if it's launched from command line or batch file regardless of this option

--replace-archive
 forces deletion of original archive(if this parameter is not passed and archive exists, returns error)
 you cannot omit archive filename with this option,
 first filename on the command line is treated as archive filename, regardless of what kind of file it is

--force-recompress
 recompresses 7z archives even if they are already torrentzipped

--strip-filenames
 do not store filename in the archive, if there is only one file in the archive
 if there are multiple files, this option is ignored

   this option could be very useful for romsets which contain only one file per set,
   you will never have to recompress anything. I'm actually sick of recompressing my nds no-intro
   romset every time some old japanese roms are renamed :) regular 7z will unpack these files
   without any problem(it's a feature of 7z that's just not enabled by default), but winrar
   will give you some trouble (it will unpack them into subdirectory structure, just try it).
   note that archives containing only one file created with this option "on" will differ
   from the same archives created with this option "off", t7z handles this automatically, so be sure to
   use this option consistently(or not use it at all).
   for archives that contain multiple files this option is ignored, so archives will be the same.
