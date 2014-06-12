>bba	
"busybox audio toolkit\n"
"usage:\n"
"bba [-h|--help]\n"
"    <command> [args]\n"
"\n"
"The most commonly used commands are:\n"
"    gen         generate tone or special signal\n"
"    play        play music file with supported decode type\n"
"    record      record sound till time limit or size limit or user interrupt\n"
"    conv        codec/width/src/channel/gain convert\n"
"    info        show current hardware informations\n"
"\n"
"See 'bba <command> for more information on a specific command.\n"

>bba gen
"busybox audio generator\n"
"usage:\n"
"bba gen [-h|--help]\n"
"        [-l|--list]\n"
"        [-v|--verbose]\n"
"        [-o <filename>] [--scale <value>] [--length <value>]\n"
"        [--width <value>] [--rate <value>] [--channel <value>]\n"
"        [--freq <value>] [--rampup <value>]\n"
"        [--tone] [--sweep <value pair>] [--whitenoise]\n"
"        [--section <value pairs>]\n"
"description:\n"
"    -h  --help\n"
"    -l  --list                  list supported encoder.\n"
"    -v  --verbose               enable verbose print.\n"
"    -o <filename>               output file name.\n"
"        --scale <value>         volume control, dBFS: 0, -3.3, -12, etc.\n"
"        --length <value>        seconds: 1, 600, etc.\n"
"\n"
"        --width <value>         bits: 8, 16, etc.\n"
"        --rate <value>          Hz: 8000, 44100, etc.\n"
"        --channel <value>       number: 1, 2, etc.\n"
"\n"
"        --freq <value>          Hz: 300, 1000, etc.\n"
"        --rampup <value>        ramp up time when gain=MAX: 5, 10, etc.\n"
"        --tone                  genarate tone of sin wav\n"
"        --sweep <value pair>    start freq, end freq: 20,22000, etc.\n"
"        --whitenoise            generate white noise.\n"
"        --section <value pairs> generate sections with demanded time & gain.\n"
"                                the (start time, gain) pairs be seperated by\n"
"                                '/': 0,12/5,0/7,12, etc. the number of \n"
"                                section is limited by SECTION_PAIRS.\n"

>bba play
"busybox audio player\n"
"usage:\n"
"bba play [-h|--help]\n"
"         [-l|--list]\n"
"         [-v|--verbose]\n"
"         [-i <filename>] [--scale <value>]\n"
"description:\n"
"    -h  --help\n"
"    -l  --list                  list supported decoder.\n"
"    -v  --verbose               enable verbose print.\n"
"    -i <filename>               input file name\n"
"        --scale <value>         volume control, dBFS: 0, -3.3, -12, etc.\n"

>bba record
"busybox audio recorder\n"
"usage:\n"
"bba record [-h|--help]\n"
"           [-l|--list]\n"
"           [-v|--verbose]\n"
"           [-o <filename>] [--length <value>] [--size <value>]\n"
"           [--width <value>] [--rate <value>] [--channel <value>]\n"
"    -h  --help\n"
"    -l  --list                  list supported encoder.\n"
"    -v  --verbose               enable verbose print.\n"
"    -o <filename>               output file name\n"
"        --length <value>        seconds: 1, 600, etc.\n"
"        --size <value>          limited record bytes: 1024, 8k, etc.\n"
"\n"
"        --width <value>         bits: 8, 16, etc.\n"
"        --rate <value>          Hz: 8000, 44100, etc.\n"
"        --channel <value>       number: 1, 2, etc.\n"

>bba conv
"usage:\n"
"bba conv [-h|--help]\n"
"         [-l|--list]\n"
"         [-v|--verbose]\n"
"         <filename> [-o <filename>]\n"
"                    [--width <value>] [--rate <value>] [--channel <value>]\n"
"                    [--enhance] [--vol <value>]\n"
"    -h  --help\n"
"    -l  --list                  list supported encoder/decoder.\n"
"    -v  --verbose               enable verbose print.\n"
"    -o <filename>\n"
"\n"
"        --width <value>         bits: 8, 16, etc.\n"
"        --rate <value>          Hz: 8000, 44100, etc.\n"
"        --channel <value>       number: 1, 2, etc.\n"
"\n"
"        --enhance               ALC(audio level control) effect.\n"
"        --vol <value>           volume control, dB: -3, 3, etc.\n"

>bba info
"busybox audio information\n"
"usage:\n"
"bba info [-h|--help]\n"
"         [-v|--verbose]\n"
"    -h  --help\n"
"    -v  --verbose               enable verbose print.\n"

//get default setting from bba.config
//files:
//	bba.h
//	bba_main.c
//	bba_misc.c
//	bba_gen.c
//	bba_play.c
//	bba_record.c
//	bba_conv.c
//	bba_info.c
//	bba.config
//	Makefile
//  Readme.txt

