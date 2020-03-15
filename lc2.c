/*********************************************************************
*
* File      : lc.c
*
* Author    : Barry Kimelman
*
* Created   : November 1, 2019
*
* Purpose   : Display files and directory names in groups
*
*********************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<dirent.h>
#include	<string.h>
#include	<time.h>
#include	<strings.h>
#include	<stdarg.h>
#include	<getopt.h>
#include	<signal.h>    /* signal name macros, and the signal() prototype */

#define	EQ(s1,s2)	(strcmp(s1,s2)==0)
#define	NE(s1,s2)	(strcmp(s1,s2)!=0)
#define	GT(s1,s2)	(strcmp(s1,s2)>0)
#define	LT(s1,s2)	(strcmp(s1,s2)<0)
#define	LE(s1,s2)	(strcmp(s1,s2)<=0)

#define		MAX_FILES	2048

typedef	struct filedata_tag {
	char	*filename;
	int		file_size;
	int		file_nlinks;
	time_t	file_mtime;
	unsigned short int	file_mode;
} FILEDATA;

typedef struct fileinfo_tag {
	int		count;
	int		maxlen;
	FILEDATA	files[MAX_FILES];
} FILEINFO;

static	struct _stat	filestats;
static	int		opt_d = 0 , opt_h = 0 , opt_t = 0;
static	int		num_args;
static	int		max_line_width = 118;
static	FILEINFO	dirs_list;
static	FILEINFO	files_list;
static	FILEINFO	char_list;
static	FILEINFO	fifo_list;
static	FILEINFO	misc_list;

extern	int		optind , optopt , opterr;

extern	void	system_error() , quit() , die();
extern	void format_number_with_commas(int number, char *buffer);

/* first, here is the signal handler */
void catch_int(int sig_num)
{
    /* re-set the signal handler again to catch_int, for next time */
    signal(SIGINT, catch_int);
    printf("Don't do that\n");
    fflush(stdout);
} /* end of catch_int */

/* here is another signal handler */
void catch_sig(int sig_num)
{
	 fflush(stderr);
	 fflush(stdout);
    /* re-set the signal handler to default action */
    signal(sig_num, SIG_DFL);
    fprintf(stderr,"Caught signal %d\n",sig_num);
    fflush(stderr);
	 /* abort(); */
	 exit(sig_num);  /* just in case abort() does not work as expected */
} /* end of catch_int */

/* string comparison function for qsort */

static int compare_name(const void *arg_s1, const void *arg_s2)
{
	const FILEDATA *f1 = (const FILEDATA *) arg_s1;
	const FILEDATA *f2 = (const FILEDATA *) arg_s2;
			
	return(_strcasecmp(f1->filename, f2->filename));
}

/* integer time comparison function for qsort */

static int compare_time(const void *arg_s1, const void *arg_s2)
{
	const FILEDATA *f1 = (const FILEDATA *) arg_s1;
	const FILEDATA *f2 = (const FILEDATA *) arg_s2;
			
	if ( f1->file_mtime < f2->file_mtime ) {
		return -1;
	}
	else {
		if ( f1->file_mtime == f2->file_mtime ) {
			return 0;
		}
		else {
			return 1;
		}
	}
}

/*********************************************************************
*
* Function  : debug_print
*
* Purpose   : Display an optional debugging message.
*
* Inputs    : char *format - the format string (ala printf)
*             ... - the data values for the format string
*
* Output    : the debugging message
*
* Returns   : nothing
*
* Example   : debug_print("The answer is %s\n",answer);
*
* Notes     : (none)
*
*********************************************************************/

void debug_print(char *format,...)
{
	va_list ap;

	if ( opt_d ) {
		va_start(ap,format);
		vfprintf(stdout, format, ap);
		fflush(stdout);
		va_end(ap);
	} /* IF debug mode is on */

	return;
} /* end of debug_print */

/*********************************************************************
*
* Function  : usage
*
* Purpose   : Display a program usage message
*
* Inputs    : char *pgm - name of program
*
* Output    : the usage message
*
* Returns   : nothing
*
* Example   : usage("The answer is %s\n",answer);
*
* Notes     : (none)
*
*********************************************************************/

void usage(char *pgm)
{
	fprintf(stderr,"Usage : %s [-dh] [-w max_line_width]\n\n",pgm);
	fprintf(stderr,"d - invoke debugging mode\n");
	fprintf(stderr,"w max_line_width - override maximum line width\n");
	fprintf(stderr,"h - produce this summary\n");

	return;
} /* end of usage */

/*********************************************************************
*
* Function  : add_file_to_list
*
* Purpose   : Add a new entry to the list of files.
*
* Inputs    : char *filename - name of file
*             FILEINFO *list - pointer to list structure
*
* Output    : (none)
*
* Returns   : (nothing)
*
* Example   : add_file_to_list(filename,&dirs_list);
*
* Notes     : (none)
*
*********************************************************************/

void add_file_to_list(char *filename, FILEINFO *list)
{
	int		len;

	debug_print("add_file_to_list(%s)\n",filename);
	if ( ++list->count > MAX_FILES ) {
		die(1,"Files maximum of %d has been exceeded\n",MAX_FILES);
	}

	list->files[list->count-1].filename = _strdup(filename);
	if ( list->files[list->count-1].filename == NULL ) {
		quit(1,"strdup failed for filename");
	}
	len = strlen(filename);
	if ( len > list->maxlen ) {
		list->maxlen = len;
	}
	list->files[list->count-1].file_mtime = filestats.st_mtime;
	list->files[list->count-1].file_size = filestats.st_size;
	list->files[list->count-1].file_nlinks = filestats.st_nlink;
	list->files[list->count-1].file_mode = filestats.st_mode;

	return;
} /* end of add_file_to_list */

/*********************************************************************
*
* Function  : list_directory
*
* Purpose   : List the files under a directory.
*
* Inputs    : char *dirname - name of directory
*
* Output    : (none)
*
* Returns   : (nothing)
*
* Example   : list_directory(dirname);
*
* Notes     : (none)
*
*********************************************************************/

int list_directory(char *dirpath)
{
	_DIR	*dirptr;
	struct _dirent	*entry;
	char	*name , filename[1024];
	unsigned short	filemode;

	debug_print("list_directory(%s)\n",dirpath);

	dirptr = _opendir(dirpath);
	if ( dirptr == NULL ) {
		quit(1,"_opendir failed for '%s'",dirpath);
	}

	entry = _readdir(dirptr);
	for ( ; entry != NULL ; entry = _readdir(dirptr) ) {
		name = entry->d_name;
		debug_print("list_directory(%s) found file '%s'\n",dirpath,name);
		sprintf(filename,"%s\\%s",dirpath,name);
		if ( _stat(filename,&filestats) < 0 ) {
			system_error("stat() failed for '%s'",filename);
		} /* IF */
		else {
			filemode = filestats.st_mode & _S_IFMT;
			switch (filemode) {
			case _S_IFDIR:
				add_file_to_list(name,&dirs_list);
				break;
			case _S_IFCHR:
				add_file_to_list(name,&char_list);
				break;
			case _S_IFIFO:
				add_file_to_list(name,&fifo_list);
				break;
			case _S_IFREG:
				add_file_to_list(name,&files_list);
				break;
			default:
				add_file_to_list(name,&misc_list);
				break;
			} /* SWITCH */

		} /* ELSE */
	} /* FOR */
	debug_print("list_directory(%s) ; all entries processed\n",dirpath);
	_closedir(dirptr);

	return(0);
} /* end of list_directory */

/*********************************************************************
*
* Function  : dump_list
*
* Purpose   : Display the contents of the specified class structure.
*
* Inputs    : list_ptr - pointer to list structure
*             title - title for dump
*
* Output    : (none)
*
* Returns   : (nothing)
*
* Example   : dump_list(&dirs_list,"Directories");
*
* Notes     : (none)
*
*********************************************************************/

void dump_list(FILEINFO *list_ptr, char *title)
{
	int		line_width , width , index;

	debug_print("dump_list(%s) count = %d\n",title,list_ptr->count);
	if ( list_ptr->count <= 0 ) {
		return;
	}
	line_width = 0;
	printf("\n");
	debug_print("dump_list() : call standout_print()\n");
	printf("%s [%d]\n",title, list_ptr->count);
	fflush(stdout);
	width = list_ptr->maxlen + 1;

	for ( index = 0 ; index < list_ptr->count ; ++index ) {
		if ( line_width+width > max_line_width ) {
			printf("\n");
			line_width = 0;
		}
		printf("%-*.*s",width,width,list_ptr->files[index].filename);
		line_width += width;
	} /* FOR */
	printf("\n");

	return;
} /* end of dump_list */

/*********************************************************************
*
* Function  : main
*
* Purpose   : program entry point
*
* Inputs    : argc - number of parameters
*             argv - list of parameters
*
* Output    : (none)
*
* Returns   : (nothing)
*
* Example   : qsort1
*
* Notes     : (none)
*
*********************************************************************/

int main(int argc, char *argv[])
{
	int		errflag , c , index , last , half;
	char	*filename , buffer[100];

	errflag = 0;
	while ( (c = _getopt(argc,argv,":dhtw:")) != -1 ) {
		switch (c) {
		case 'h':
			opt_h = 1;
			break;
		case 'd':
			opt_d = 1;
			break;
		case 't':
			opt_t = 1;
			break;
		case 'w':
			max_line_width = atoi(optarg);
			break;
		case '?':
			printf("Unknown option '%c'\n",optopt);
			errflag += 1;
			break;
		case ':':
			printf("Missing value for option '%c'\n",optopt);
			errflag += 1;
			break;
		default:
			printf("Unexpected value from getopt() '%c'\n",c);
		} /* SWITCH */
	} /* WHILE */
	if ( errflag ) {
		usage(argv[0]);
		die(1,"\nAborted due to parameter errors\n");
	} /* IF */
	if ( opt_h ) {
		usage(argv[0]);
		exit(0);
	} /* IF */

	dirs_list.count = 0;
	dirs_list.maxlen = 0;

	files_list.count = 0;
	files_list.maxlen = 0;

	char_list.count = 0;
	char_list.maxlen = 0;

	fifo_list.count = 0;
	fifo_list.maxlen = 0;

	misc_list.count = 0;
	misc_list.maxlen = 0;

    /* set the INT (Ctrl-C) signal handler to 'catch_int' */
    signal(SIGINT, catch_int);
    /* signal(SIGBUS, catch_sig); */
    signal(SIGSEGV, catch_sig);

	num_args = argc - optind;
	if ( num_args <= 0 ) {
		list_directory(".");
	} /* IF */
	else {
		filename = argv[optind];
		list_directory(filename);
	} /* ELSE */

	if ( opt_t ) {
		qsort(dirs_list.files, (unsigned) dirs_list.count, sizeof(FILEDATA), compare_time);
		qsort(files_list.files, (unsigned) files_list.count, sizeof(FILEDATA), compare_time);
		qsort(char_list.files, (unsigned) char_list.count, sizeof(FILEDATA), compare_time);
		qsort(fifo_list.files, (unsigned) fifo_list.count, sizeof(FILEDATA), compare_time);
		qsort(misc_list.files, (unsigned) misc_list.count, sizeof(FILEDATA), compare_time);
	} /* IF */
	else {
		qsort(dirs_list.files, (unsigned) dirs_list.count, sizeof(FILEDATA), compare_name);
		qsort(files_list.files, (unsigned) files_list.count, sizeof(FILEDATA), compare_name);
		qsort(char_list.files, (unsigned) char_list.count, sizeof(FILEDATA), compare_name);
		qsort(fifo_list.files, (unsigned) fifo_list.count, sizeof(FILEDATA), compare_name);
		qsort(misc_list.files, (unsigned) misc_list.count, sizeof(FILEDATA), compare_name);
	} /* ELSE */
	dump_list(&files_list,"Files");
	dump_list(&dirs_list,"Directories");
	dump_list(&char_list,"Character Devices");
	dump_list(&misc_list,"Misc");
	dump_list(&fifo_list,"FIFO");

	exit(0);
} /* end of main */
