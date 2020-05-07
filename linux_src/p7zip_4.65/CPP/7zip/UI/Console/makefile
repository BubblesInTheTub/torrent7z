PROG=../../../../bin/t7z

LOCAL_FLAGS=\
  -DCOMPRESS_MT \
  -DEXTERNAL_LZMA \
  -DEXTERNAL_CODECS \
  -DBREAK_HANDLER \
  -DBENCH_MT \
  -D_NO_CRYPTO \
  -DWIN_LONG_PATH \
  -DUNICODE \
  -D_UNICODE \
  -DBUILD_SHARED_LIB=OFF

include ../../../../makefile.crc32
include ../../../../makefile.machine

LIBS=$(LOCAL_LIBS_DLL)

CONSOLE_OBJS = \
  ConsoleClose.o \
  ExtractCallbackConsole.o \
  List.o \
  Main.o \
  MainAr.o \
  OpenCallbackConsole.o \
  PercentPrinter.o \
  UpdateCallbackConsole.o \
  UserInputUtils.o \

# NewHandler.o
COMMON_OBJS = \
  CommandLineParser.o \
  CRC.o \
  IntToString.o \
  ListFileUtils.o \
  StdInStream.o \
  StdOutStream.o \
  MyString.o \
  MyWindows.o \
  myGetTickCount.o \
  StringConvert.o \
  StringToInt.o \
  UTFConvert.o \
  MyVector.o \
  Wildcard.o \

# MemoryLock.o Registry.o
WIN_OBJS = \
  DLL.o \
  Error.o \
  FileDir.o \
  FileFind.o \
  FileIO.o \
  FileName.o \
  PropVariant.o \
  PropVariantConversions.o \
  System.o \
  Time.o \

7ZIP_COMMON_OBJS = \
  FilePathAutoRename.o \
  FileStreams.o \
  ProgressUtils.o \
  StreamUtils.o \
  7zHeader.o \

# WorkDir.o
UI_COMMON_OBJS = \
  ArchiveCommandLine.o \
  ArchiveExtractCallback.o \
  ArchiveOpenCallback.o \
  DefaultName.o \
  EnumDirItems.o \
  Extract.o \
  ExtractingFilePath.o \
  LoadCodecs.o \
  OpenArchive.o \
  PropIDUtils.o \
  SetProperties.o \
  SortUtils.o \
  TempFiles.o \
  Update.o \
  UpdateAction.o \
  UpdateCallback.o \
  UpdatePair.o \
  UpdateProduce.o \

LZMA_BENCH_OBJS = \
  LzmaBench.o \
  LzmaBenchCon.o \

T7Z_OBJS = \
  ../../../../torrent7z/t7z.o \
  ../../../../torrent7z/torrent7z_signature.o \
  ../../../../torrent7z/w32t.o \
  ../../../../torrent7z/usage.o

# Alloc.o
C_OBJS = \
  Threads.o \

OBJS = \
  mySplitCommandLine.o \
  myAddExeFlag.o \
  wine_date_and_time.o \
  $(CONSOLE_OBJS) \
  $(COMMON_OBJS) \
  $(WIN_OBJS) \
  $(7ZIP_COMMON_OBJS) \
  $(UI_COMMON_OBJS) \
  CopyCoder.o \
  $(LZMA_BENCH_OBJS) \
  $(C_OBJS) \
  $(OBJ_CRC32) \
  $(T7Z_OBJS)

include ../../../../makefile.glb

