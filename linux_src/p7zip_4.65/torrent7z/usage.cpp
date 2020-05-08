/*  #########################################################################  */

#include "StdAfx.h"
#include "t7z.h"

/*  #########################################################################  */

namespace torrent7z
{

/*  #########################################################################  */

TCHAR*usage_info=\
L"Usage: t7z [<command>] [<switches>...] [<archive_name>] [<file_names>...]\n" \
L"       [<@listfiles...>]\n" \
L"\n" \
L"<Commands>\n" \
L"  no command: convert specified files to t7z\n" \
L"  a: Add files to archive\n" \
L"  d: Delete files from archiveonly with --cmpro switch\n" \
L"  e: Extract files from archive without directory names\n" \
L"  l: List contents of archive\n" \
L"  t: Test integrity of archive\n" \
L"  x: eXtract files with full paths\n" \
L"\n" \
L"<Switches>\n" \
L"  -K: Keep the original extension when creating archive name\n" \
L"  -ba: Disable copyright info\n" \
L"  -bd: Disable percentage indicator\n" \
L"  -o{Directory}: set Output directory\n" \
L"  -aoa: Overwrite All existing files without prompt\n" \
L"  -aos: Skip extracting of existing files\n" \
L"  -aou: auto rename extracted file\n" \
L"  -aot: auto rename existing file\n" \
L"  -scs{UTF-8 | WIN | DOS}: set charset for list files\n" \
L"  -slt: show technical information for l List command\n" \
L"  -ssc[-]: set sensitive case mode\n" \
L"  -y: assume Yes on all queries\n" \
L"  -n: assume No on all queries\n" \
L"  --default-priority, -dp: use priority of the calling process for operation\n" \
L"  --log{logfile}, -l{logfile}: log messages to specified logfile\n" \
L"  --batch, -b: do not pause at the end of operation\n" \
L"  --cmpro, -cm: create non-solid non-t7zipped archive\n" \
L"  --replace-archive, -ra: force deletion of the existing archive\n" \
L"  --force-recompress, -fr: recompress all 7z archives, including t7zipped\n" \
L"  --strip-filenames, -sf: do not store filename in the archive\n";

/*  #########################################################################  */

}
