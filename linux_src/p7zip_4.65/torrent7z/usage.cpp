/*  #########################################################################  */

#include "StdAfx.h"
#include "t7z.h"

/*  #########################################################################  */

namespace torrent7z
{

/*  #########################################################################  */

TCHAR*usage_info=\
text("Usage: t7z [<command>] [<switches>...] [<archive_name>] [<file_names>...]\n") \
text("       [<@listfiles...>]\n") \
text("\n") \
text("<Commands>\n") \
text("  no command: convert specified files to t7z\n") \
text("  a: Add files to archive\n") \
text("  d: Delete files from archive(only with --cmpro switch)\n") \
text("  e: Extract files from archive (without directory names)\n") \
text("  l: List contents of archive\n") \
text("  t: Test integrity of archive\n") \
text("  x: eXtract files with full paths\n") \
text("\n") \
text("<Switches>\n") \
text("  -ba: Disable copyright info\n") \
text("  -bd: Disable percentage indicator\n") \
text("  -o{Directory}: set Output directory\n") \
text("  -aoa: Overwrite All existing files without prompt\n") \
text("  -aos: Skip extracting of existing files\n") \
text("  -aou: auto rename extracted file\n") \
text("  -aot: auto rename existing file\n") \
text("  -scs{UTF-8 | WIN | DOS}: set charset for list files\n") \
text("  -slt: show technical information for l (List) command\n") \
text("  -ssc[-]: set sensitive case mode\n") \
text("  -y: assume Yes on all queries\n") \
text("  -n: assume No on all queries\n") \
text("  --default-priority, -dp: use priority of the calling process for operation\n") \
text("  --log{logfile}, -l{logfile}: log messages to specified logfile\n") \
text("  --batch, -b: do not pause at the end of operation\n") \
text("  --cmpro, -cm: create non-solid non-t7zipped archive\n") \
text("  --replace-archive, -ra: force deletion of the existing archive\n") \
text("  --force-recompress, -fr: recompress all 7z archives, including t7zipped\n") \
text("  --strip-filenames, -sf: do not store filename in the archive\n");

/*  #########################################################################  */

}