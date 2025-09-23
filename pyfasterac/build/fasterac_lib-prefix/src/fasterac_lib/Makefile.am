
SUBDIRS                   = lib src include man

confexecdir	     	        = ${bindir}
confexec_SCRIPTS          = fasterac_config.sh  \
                            fasterac_config.csh \
						          fasterac_examples_copy

pkgconfigdir              = ${libdir}/pkgconfig
pkgconfig_DATA            = libfasterac.pc

dataExampledir            = ${prefix}/share/fasterac/data
dist_dataExample_DATA     = data/*.fast

readerExampledir          = ${prefix}/share/fasterac/examples/data_reader
dist_readerExample_DATA   = examples/data_reader/README   \
									 examples/data_reader/Makefile \
									 examples/data_reader/*.c
dist_readerExample_SCRIPTS= examples/data_reader/TEST.sh

rootTreeExampledir        = ${prefix}/share/fasterac/examples/faster_to_root
dist_rootTreeExample_DATA = examples/faster_to_root/Makefile         \
									 examples/faster_to_root/group2tree.C     \
									 examples/faster_to_root/group2tree_bis.C \
									 examples/faster_to_root/regroup2tree.C   \
									 examples/faster_to_root/rootlogon.C
dist_rootTreeExample_SCRIPTS = examples/faster_to_root/TEST.sh

miscExampledir            = ${prefix}/share/fasterac/examples/misc
dist_miscExample_DATA     = examples/misc/Makefile        \
									 examples/misc/*.c
dist_miscExample_SCRIPTS  = examples/misc/TEST.sh

oscilloExampledir         = ${prefix}/share/fasterac/examples/oscillo
dist_oscilloExample_DATA  = examples/oscillo/Makefile     \
									 examples/oscillo/*.c
dist_oscilloExample_SCRIPTS = examples/oscillo/TEST.sh

oscirootExampledir        = ${prefix}/share/fasterac/examples/osciroot
dist_oscirootExample_DATA = examples/osciroot/README    \
									 examples/osciroot/*.h       \
									 examples/osciroot/*.C
dist_oscirootExample_SCRIPTS = examples/osciroot/clean.sh \
                               examples/osciroot/TEST.sh

libsrcdir                 = ${prefix}/share/fasterac/src/lib
dist_libsrc_DATA          = lib/fasterac.c     \
									 lib/fast_data.c    \
									 lib/farray.c       \
									 lib/utils.c        \
									 lib/qdc.c          \
									 lib/adc.c          \
									 lib/spectro.c      \
									 lib/rf.c           \
									 lib/electrometer.c \
									 lib/scaler.c       \
									 lib/sampling.c     \
									 lib/qtdc.c         \
									 lib/plas.c         \
									 lib/group.c        \
									 lib/sampler.c      \
									 lib/smart.c        \
									 lib/jdb_hv.c

incsrcdir                 = ${prefix}/share/fasterac/src/lib/fasterac
dist_incsrc_DATA          = include/fasterac/fasterac.h     \
									 include/fasterac/fast_data.h    \
									 include/fasterac/farray.h       \
									 include/fasterac/utils.h        \
									 include/fasterac/qdc.h          \
									 include/fasterac/adc.h          \
									 include/fasterac/spectro.h      \
									 include/fasterac/rf.h           \
									 include/fasterac/electrometer.h \
									 include/fasterac/scaler.h       \
									 include/fasterac/qt2t.h         \
									 include/fasterac/sampler.h      \
									 include/fasterac/sampling.h     \
									 include/fasterac/qtdc.h         \
									 include/fasterac/jdb_hv.h       \
									 include/fasterac/adc_caras.h    \
									 include/fasterac/qdc_caras.h    \
									 include/fasterac/plas.h         \
									 include/fasterac/smart.h         \
									 include/fasterac/group.h        \
									 include/fasterac/rf_caras.h

binsrcdir                 = ${prefix}/share/fasterac/src/prog
dist_binsrc_DATA          = src/faster_disfast.c        \
									 src/faster_file_is_sorted.c \
									 src/faster_file_sort.c      \
									 src/faster_file_ungroup.c

dmosrcdir                 = ${prefix}/share/fasterac/src/dmo
dist_dmosrc_DATA          = src/dmo/autoreader.make.in        \
									 src/dmo/faster_reader_demo_code.c.in

EXTRA_DIST = saved_svn_version.txt

