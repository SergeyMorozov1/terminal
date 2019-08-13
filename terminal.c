
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <curl/curl.h>
#if defined( GDK_WINDOWING_X11 )
#include <gdk/gdkx.h>
#endif
#include <gst/gst.h>
#include <gst/gstbus.h>
#include <gst/video/videooverlay.h>
#include <libintl.h>
#include <locale.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <uuid/uuid.h>
#include "nxjson.h"


#define DEBUGGING 1

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define APP_VERSION 112
#define LOGFILE_PATH "./log/log.txt"
#define LOGFILE_PATH_ERRORS "./log/errors.txt"
#define TRANSACTION_LOG_DIRECTORY "./log/"
#define TRANSACTION_PATH "./conf/trans"
#define SERVICE_CARDS_PATH "./conf/cards"
#define SETTINGS_PATH "./conf/settings"
#define SETTINGS_PATH "./conf/settings"
#ifndef __arm__
#define INTRO_KZ_PATH "./media/all_kaz.avi"
#define INTRO_RU_PATH "./media/all_rus.avi"
#else
#define INTRO_KZ_PATH "./media/all_kaz.flv"
#define INTRO_RU_PATH "./media/all_rus.flv"
#endif
#define DOWNLOADS_PATH "./downloads/terminal.tgz"
#define UPDATE_PATH "./update/terminal.tgz"

#define APP_WIDTH 1024
#define APP_HEIGHT 600
#define CHEQUE_WIDTH 240
#define CHEQUE_HEIGHT 320

#define HINT_OFFSET 240
#define LABEL_OFFSET 600

#define PACKAGE "terminal"
#ifndef __arm__
	#define PACKAGE_LOCALE_DIR "/usr/share/locale"
#else
	#define PACKAGE_LOCALE_DIR "/usr/lib/locale"
#endif
#define _(String) dgettext(PACKAGE, String)
#define BTN_WIDTH APP_WIDTH * 0.33333
#define BTN_HEIGHT BTN_WIDTH * 0.4
#define NUM_BTN_SIZE APP_WIDTH * 0.09

#define MAX_COUNTER_VALUE 120
#define INTRO_PLAY_INTERVAL_SECONDS 60
#define INACTIVITY_SECONDS 30
#define COUNTDOWN_SECONDS 30
#define AUTOCOMPLETE_SECONDS 7
#define DISPENSER_STATUS_SECONDS 30
#define PRINTER_STATUS_SECONDS 30
#define POLYNOMIAL 0x08408
#define NETWORK_REQUEST_INTERVAL_SECONDS 30
#define TRANSACTION_RESEND_INTERVAL_SECONDS 30

#define TERMINAL_ID_URL "https://onay.guidejet.kz/Terminal/GetId/"
#define TERMINAL_INFO_URL "https://onay.guidejet.kz/Terminal/GetInfo/"
#define TERMINAL_STATUS_URL "https://onay.guidejet.kz/Terminal/ReportStatus/"
#define TERMINAL_UPDATE_URL "https://onay.guidejet.kz/Terminal/GetUpdate/"
#define CARD_INFO_URL "https://onay.guidejet.kz/Transaction/CardInfo/"
#define REFILL_BALANCE_URL "https://onay.guidejet.kz/Transaction/RefillBalance/"
#define BUY_CARD_URL "https://onay.guidejet.kz/Transaction/BuyCard/"
#define BUY_PASS_URL "https://onay.guidejet.kz/Transaction/BuyPass/"
#define COLLECT_URL "https://onay.guidejet.kz/Transaction/Encash/"


enum APP_STATE { STATE_IDLE, STATE_BUY_CARD, STATE_BUY_PASS, STATE_CHECK_REFILL, STATE_COLLECT, STATE_MAINTENANCE, STATE_OUT_OF_SERVICE };
enum APP_SCREEN { SCREEN_INTRO, SCREEN_HOME, SCREEN_NO_CARDS, SCREEN_PASS_INTRO, SCREEN_PASS_FIRST, SCREEN_PASS_ALREADY_ACTIVE, SCREEN_PASS_SUFFICIENT_BALANCE, SCREEN_PASS_PARTIAL_AMOUNT, SCREEN_PASS_SUFFICIENT_AMOUNT, SCREEN_PASS_INSUFFICIENT_AMOUNT, SCREEN_PASS_ACTIVE, SCREEN_PASS_ACTIVE_NEXT_MONTH, SCREEN_PASS_ACTIVE_WHEN_CONNECTED, SCREEN_CARD_INTRO, SCREEN_CARD_PARTIAL_AMOUNT, SCREEN_CARD_SUFFICIENT_AMOUNT, SCREEN_CARD_INSUFFICIENT_AMOUNT, SCREEN_CARD_BOUGHT, SCREEN_CARD_FREE, SCREEN_CARD_PRE_SCAN, SCREEN_CARD_SCAN, SCREEN_LOADING, SCREEN_BALANCE_INTRO, SCREEN_BALANCE_REFILL, SCREEN_CARD_NOT_FOUND, SCREEN_BALANCE_REFILLED, SCREEN_BALANCE_REFILLED_WHEN_CONNECTED, SCREEN_OUT_OF_SERVICE, SCREEN_MORE_TIME, SCREEN_PIN_CODE, SCREEN_MANUAL_INPUT };
enum APP_LANG { LANG_KZ, LANG_RU, LANG_EN };
enum CARD_STATUS { CARD_OK, CARD_NOT_FOUND, CARD_NETWORK_ERROR };
enum BILL_ACCEPTOR_STATUS { BILL_ACCEPTOR_UNKNOWN = 0x00, POWER_UP = 0x10, POWER_UP_BILL_VALIDATOR = 0x11, POWER_UP_BILL_STACKER = 0x12, INITIALIZE = 0x13, IDLING = 0x14, ACCEPTING = 0x15, STACKING = 0x17, RETURNING = 0x18, DISABLED = 0x19, HOLDING = 0x1A, BUSY = 0x1B, REJECTING = 0x1C, DROP_FULL = 0x41, DROP_OUT_OF_POS = 0x42, VALIDATOR_JAMMED = 0x43, CASSETTE_JAMMED = 0x44, CHEATED = 0x45, PAUSE = 0x46, FAILURE = 0x47, ESCROW_POS = 0x80, BILL_STACKED = 0x81, BILL_RETURNED = 0x82 };
enum CARD_DISPENSER_STATUS { DISPENSER_NOT_READY = 0x00, DISPENSER_READY = 0x30, DISPENSER_DISPENSING = 0x31, DISPENSER_UNKNOWN_ERROR = 0x32, DISPENSER_DISPENSING_SUCCESS = 0x33, DISPENSER_DISPENSING_ERROR = 0x34, DISPENSER_PRE_DISPENSE = 0x35, DISPENSER_EXECUTION_ERROR = 0x36, DISPENSER_CHECK_STATUS = 50 };
enum CARD_STACKER_STATUS { STACK_UNKNOWN = 0x00, STACK_FULL = 0x30, STACK_LOW = 0x31, STACK_EMPTY = 0x32 };
enum CARD_READER_STATUS { READER_UNKNOWN = 0x00, READER_OK = 0x01, READER_ERROR = 0x02, READER_MSAM_DRIVER_ERROR = 0x10, READER_MSAM_POWER_ERROR = 0x11, READER_MSAM_SELECT_ERROR = 0x12, READER_PIN_REQUIRED = 0x14, READER_WRONG_PIN = 0x15, READER_MSAM_BLOCKED = 0x16, READER_MSAM_DATA_EXCHANGE_ERROR = 0x17, READER_MSAM_UNEXPECTED_ERROR = 0x18, READER_CRC_ERROR = 0x20, READER_SERIAL_REQUIRED = 0x21, READER_PICC_AUTH_ERROR = 0x30, READER_PICC_ERROR = 0x31, READER_BLOCKED_TAMP = 0x44, READER_BLOCKED_MEMC = 0x45 };
enum PRINTER_STATUS { PRINTER_OK, PRINTER_NOT_READY, PRINTER_OFFLINE };
enum PRINTER_PAPER_STATUS { PAPER_UNKNOWN = 0x00, PAPER_FULL = 0x01, PAPER_LOW = 0x02, PAPER_EMPTY = 0x03 };
enum TRANSACTION_TYPE { TRANSACTION_REFILL, TRANSACTION_BUY_CARD, TRANSACTION_BUY_PASS, TRANSACTION_COLLECT };
enum TRANSACTION_STATUS { TRANSACTION_INITIAL, TRANSACTION_FISCALIZED, TRANSACTION_NOT_FISCALIZED, TRANSACTION_COMPLETE, TRANSACTION_FAILED };
enum NETWORK_EVENT { NE_METADATA_LOADED, NE_CARD_INFO_LOADED, NE_CARD_NOT_FOUND, NE_CONNECTION_STATUS_CHANGED, NE_TRANSACTION_COMPLETE, NE_TRANSACTION_FAILED };
enum TERMINAL_STATUS { TERMINAL_OK = 0x00, TERMINAL_LOW_CARDS = 0x01, TERMINAL_LOW_PAPER = 0x02, TERMINAL_NO_CARDS = 0x04, TERMINAL_NO_PAPER = 0x08, TERMINAL_TRANSACTION_FAILED = 0x10, TERMINAL_BILL_ACCEPTOR_FAILED = 0x20, TERMINAL_PRINTER_FAILED = 0x40, TERMINAL_DISPENSER_FAILED = 0x80, TERMINAL_CARD_READER_FAILED = 0x100 };
enum TERMINAL_KIND { KIND_SELF_SERVICE = 0x00, KIND_POS = 0x01, KIND_BARRIER_IN = 0x02, KIND_BARRIER_OUT = 0x03, KIND_POS_TOURIST = 0x04, KIND_TURNSTILE = 0x05 };
enum TERMINAL_COMMAND { CMD_NONE = 0x00, CMD_REBOOT = 0x01 };
enum UNLOCK_MODE { UNLOCK_NONE = 0x00, UNLOCK_FINANCIAL = 0x01, UNLOCK_MECHANICAL = 0x02 };


const char* const LOCALES[] = { "kk_KZ", "ru_RU", "en_US" };
const char* const MONTH_NAMES[] = { "ҚАҢТАР", "АҚПАН", "НАУРЫЗ", "СӘУIР", "МАМЫР", "МАУСЫМ", "ШIЛДЕ", "ТАМЫЗ", "ҚЫРКҮЙЕК", "ҚАЗАН", "ҚАРАША", "ЖЕЛТОҚСАН", "ЯНВАРЬ", "ФЕВРАЛЬ", "МАРТ", "АПРЕЛЬ", "МАЙ", "ИЮНЬ", "ИЮЛЬ", "АВГУСТ", "СЕНТЯБРЬ", "ОКТЯБРЬ", "НОЯБРЬ", "ДЕКАБРЬ", "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" };
const char* TRANSACTION_TYPE_NAMES[] = { "REFILL", "BUY_CARD", "BUY_PASS", "COLLECT" };
const char* const APP_SCREEN_NAMES[] = { "SCREEN_INTRO", "SCREEN_HOME", "SCREEN_NO_CARDS", "SCREEN_PASS_INTRO", "SCREEN_PASS_FIRST", "SCREEN_PASS_ALREADY_ACTIVE", "SCREEN_PASS_SUFFICIENT_BALANCE", "SCREEN_PASS_PARTIAL_AMOUNT", "SCREEN_PASS_SUFFICIENT_AMOUNT", "SCREEN_PASS_INSUFFICIENT_AMOUNT", "SCREEN_PASS_ACTIVE", "SCREEN_PASS_ACTIVE_NEXT_MONTH", "SCREEN_PASS_ACTIVE_WHEN_CONNECTED", "SCREEN_CARD_INTRO", "SCREEN_CARD_PARTIAL_AMOUNT", "SCREEN_CARD_SUFFICIENT_AMOUNT", "SCREEN_CARD_INSUFFICIENT_AMOUNT", "SCREEN_CARD_BOUGHT", "SCREEN_CARD_FREE", "SCREEN_CARD_PRE_SCAN", "SCREEN_CARD_SCAN", "SCREEN_LOADING", "SCREEN_BALANCE_INTRO", "SCREEN_BALANCE_REFILL", "SCREEN_CARD_NOT_FOUND", "SCREEN_BALANCE_REFILLED", "SCREEN_BALANCE_REFILLED_WHEN_CONNECTED", "SCREEN_OUT_OF_SERVICE", "SCREEN_MORE_TIME", "SCREEN_PIN_CODE", "SCREEN_MANUAL_INPUT" };
const int const BILL_NOMINATIONS[] = { 0, 200, 500, 1000, 2000, 5000, 10000 };

GtkWidget* main_window = NULL;
GtkWidget* curr_form = NULL;

GtkWidget* hint_labels[3] = { NULL, NULL, NULL };
GtkWidget* value_labels[3] = { NULL, NULL, NULL };
GtkWidget* instructions_label = NULL;
GtkWidget* back_button = NULL;
GtkWidget* next_button = NULL;

static guintptr video_window_handle = 0;
GstElement* pipeline;
GstElement* source;
GstBus* bus;
int times_intro_played = 0;
char intro_file_path[128];
guint counter_timer_id = 0;
guint time_counter = 0;
char input_text[32];

enum APP_STATE  curr_state = STATE_IDLE;
enum APP_SCREEN curr_screen = SCREEN_INTRO;
enum APP_SCREEN last_screen = SCREEN_INTRO;
enum APP_LANG lang = LANG_RU;
enum CARD_DISPENSER_STATUS dispenser_status = DISPENSER_NOT_READY;
enum CARD_STACKER_STATUS stacker_status = STACK_UNKNOWN;
enum PRINTER_PAPER_STATUS paper_status = PAPER_UNKNOWN;
enum BILL_ACCEPTOR_STATUS bill_acceptor_status = BILL_ACCEPTOR_UNKNOWN;
enum CARD_READER_STATUS card_reader_status = READER_UNKNOWN;
int printer_status = PRINTER_NOT_READY;
int dispense_attempts = 0;

GdkColor onay_color  = { 0, 0xfcfc, 0xebeb, 0x3636 }; // c * 257; #ffed00
GdkColor onay_highl_color  = { 0, 0xffff, 0xffff, 0xffff };
GdkColor black_color = { 0, 0x0000, 0x0000, 0x0000 };

PangoFontDescription* font_labels = NULL;
PangoFontDescription* font_buttons = NULL;
PangoFontDescription* font_instructions = NULL;

gboolean cash_retrieval_enabled = FALSE;
gboolean card_dispense_requested = FALSE;
gboolean print_cheque_requested = FALSE;
gboolean status_report_requested = FALSE;
int unlock_requested = UNLOCK_NONE;
int issue_sound_signal = 0;


struct card_info {
    char uid[32];
    char pan[32];
    float balance;
    float pass_price;
    float ride_price;
    time_t pass_beg;
    time_t pass_end;
    gboolean initialized;
    gboolean active;
    int type;
    gboolean pass_active;
    enum CARD_STATUS status;
} card;

struct transaction_info {
    char id[32];
    char pan[32];
    char uid[32];
    char fz_number[32];
    int amount;
    int cost;
    int type;
    time_t beg;
    time_t end;
    int status;
    gboolean active;
    gboolean receipt_printed;
    gboolean card_dispensed;
    int session_amount;
} transaction, print_transaction;

struct terminal_info {
	int id;
	int status;
	int kind;
	int update_to_version;
	int update_interval;
	char phone_number[32];
	char phone_number_2[32];
	char operating_system[256];
	char serial[32];
	char msam[32];
	char iin[32];
	char kkm[32];
	char kkm_reg_number[32];
	float card_price;
	gboolean connected;
	gboolean startup;
	enum TERMINAL_COMMAND command;
} terminal;

struct service_card_info {
	char pan[32];
	char pin[16];
	int type;
	int trials;
}* servicecards = NULL;
int servicecards_count = 0;

void reload_form( enum APP_SCREEN screen, gboolean reload_ui );
void display_form( enum APP_SCREEN screen );
void display_more_time_form( int counter );
void refresh_form();
void reset_card_info();
void reset_transaction_info();
void clear_transaction();
gboolean load_transaction();
void activate_transaction();
void load_service_cards();

gboolean counter_timer_callback( gpointer data );

void trace( char* fmt, ... )
{
    char msg[512];
    va_list vl;
    va_start( vl, fmt );
    vsnprintf( msg, sizeof(msg), fmt, vl );
    va_end( vl );

    time_t t = time( NULL );
    struct tm* tm_info = localtime( &t );
    char dt[32];
    strftime( dt, 32, "%H:%M:%S", tm_info );

	FILE *f = fopen( LOGFILE_PATH, "a" );
	if( f != NULL )
	{
		fprintf( f, "%s\t%s\n", dt, msg );
		fclose( f );
	}

	printf( "%s\t%s\n", dt, msg );
}

void trace_error( char* fmt, ... )
{
    char msg[512];
    va_list vl;
    va_start( vl, fmt );
    vsnprintf( msg, sizeof(msg), fmt, vl );
    va_end( vl );

	FILE* file = fopen( LOGFILE_PATH_ERRORS, "a" );
	if( file != NULL )
	{
		fprintf( file, "%s\n", msg );
		fclose( file );
	}

	printf( "%s\n", msg );
}

void print_hex( const char* data, unsigned int len )
{
	unsigned int i = 0;
	for( i = 0; i < len; i++ )
		printf( "%02x", data[i] );
	printf( "\n" );
}

unsigned int get_crc_16( const char* data, unsigned int len )
{
	unsigned int tmpCRC, CRC, i, j;
	CRC = 0;
	for( i = 0; i < len; i++ )
	{
		tmpCRC = CRC ^ data[i];
		for( j = 0; j < 8; j++ )
		{
			if( tmpCRC & 0x0001 )
			{
				tmpCRC >>= 1;
				tmpCRC ^= POLYNOMIAL;
			}
			else
				tmpCRC >>= 1;
		}
		CRC = tmpCRC;
	}
	return CRC;
}

gboolean check_crc( const char* data, unsigned int len )
{
	if( len > 2 )
	{
		unsigned int crc = get_crc_16( data, len - 2 );
		return ( crc == ( data[len - 2] | ( data[len - 1] << 8 ) ) );
	}
	else
		return FALSE;
}

void int_to_bytes( uint32_t num, char* bytes )
{
	uint32_t val = htonl( num );		// convert endianness
	bytes[0] = (val >> 24) & 0xff;
	bytes[1] = (val >> 16) & 0xff;
	bytes[2] = (val >> 8) & 0xff;
	bytes[3] = val & 0xff;
}

void short_to_bytes( uint16_t num, char* bytes )
{
	uint16_t val = htons( num );		// convert endianness
	bytes[0] = (val >> 8) & 0xff;
	bytes[1] = val & 0xff;
}

void substring( char s[], char sub[], int pos, int len )
{
   int c = 0;
   // int p = ( pos > 0 )? pos: 1;
   while( c < len )
   {
      // sub[c] = s[p + c - 1];
	   sub[c] = s[pos + c];
       c++;
   }
   sub[c] = '\0';
}

int index_of( char* srcstr, char* substr, int index )
{
	int ind = -1;
	int len = strlen( srcstr );
	if( len > 0 && strlen( substr ) > 0 )
	{
		if( index > 0 )
		{
			char sub[len];
			substring( srcstr, sub, index, strlen( srcstr ) );
			// printf( "Substring: %s\n", sub );

			char* ptr = strstr( sub, substr );
			if( ptr != NULL )
				ind = index + ptr - sub;
		}
		else
		{
			char* ptr = strstr( srcstr, substr );
			if( ptr != NULL )
				ind = ptr - srcstr;
		}
	}
	return ind;
}

gboolean value_substring( char* str, char* key, char val[] )
{
	int ind = index_of( str, key, 0 );
	if( ind >= 0 )
	{
		int start = ind + strlen( key );
		int ind2 = index_of( str, "\n", start );
		if( ind2 > 0 )
		{
			substring( str, val, start, ind2 - start );
			return TRUE;
		}
	}
	return FALSE;
}

char* trim( char *str )
{
	char *end;
	// Trim leading space & quotation mark
	while( isspace( (unsigned char)*str ) || ((unsigned char)*str) == '"' ) str++;

	if( *str == 0 )  // All spaces?
		return str;

	// Trim trailing space
	end = str + strlen( str ) - 1;
	while( end > str && ( isspace( (unsigned char)*end ) || ((unsigned char)*end) == '"' ) ) end--;

	// Write new null terminator
	*(end + 1) = 0;

	return str;
}

void get_mac_address( char mac[] )
{
    FILE* file = NULL;

#ifdef __arm__
    file = fopen( "/sys/class/net/eth0/address", "r" );
#else
    file = fopen( "/sys/class/net/ens33/address", "r" );
#endif

    int c = 0;
    if( file != NULL )
    {
		char line[256];
		while( fgets( line, sizeof( line ), file ) )
		{
			if( index_of( line, ":", 0 ) > -1 )
			{
				for( int i = 0; i < strlen( line ); i++ )
				{
					if( line[i] != ':' && line[i] != '\n' )
						mac[c++] = line[i];
				}
				break;
			}
		}
		fclose( file );
    }
    else
    	trace( "MAC address can not be read: %d (%s)", errno, strerror( errno ) );
    mac[c] = '\0';
}

void get_os_version( char os[] )
{
	FILE* file = popen( "/usr/bin/lsb_release -d", "r" );
	if( file != NULL )
	{
		char line[256], ver[256]; int ind = -1, ind2 = -1, start = 0, end = 0;
		fgets( line, sizeof( line ), file );

		ind = index_of( line, "escription:", 0 );
		if( ind >= 0 )
		{
			start = ind + 11;
			ind2 = index_of( line, "\n", start );
			if( ind2 > 0 )
			{
				end = ind2 - start;
				substring( line, ver, start, end );
				strcpy( os, trim( ver ) );
			}
		}
		pclose( file );
	}
}

// -------------- app ------------------------------------------------------------------->

void init_terminal_info()
{
	strcpy( terminal.phone_number, "" );
	strcpy( terminal.phone_number_2, "" );
	strcpy( terminal.iin, "801140000022" );
	strcpy( terminal.kkm, "R10100800" );
	strcpy( terminal.kkm_reg_number, "600800054208" );
	strcpy( terminal.serial, "" );
	strcpy( terminal.msam, "" );
	get_os_version( terminal.operating_system );
	get_mac_address( terminal.serial );
	terminal.connected = FALSE;
	terminal.startup = TRUE;
	terminal.kind = KIND_POS;
	terminal.command = CMD_NONE;
}

void init_locale()
{
	setlocale( LC_MESSAGES, "" );

	setenv( "LANG", LOCALES[lang], TRUE );
	setenv( "LANGUAGE", LOCALES[lang], TRUE );
	// setlocale( LC_ALL, "" );

	// setenv( "LC_MESSAGES", LOCALES[lang], TRUE );
    bindtextdomain( PACKAGE, PACKAGE_LOCALE_DIR );
    bind_textdomain_codeset( PACKAGE, "UTF-8" );
    textdomain( PACKAGE );
}

void init_settings()
{
	init_terminal_info();

    FILE* file = fopen( SETTINGS_PATH, "r" );
    if( file != NULL )
    {
		char line[256], val[8];
		int ind = -1, ind2 = -1, start = 0, end = 0;
		while( fgets( line, sizeof( line ), file ) )
		{
			ind = index_of( line, "Id=", 0 );
			if( ind >= 0 )
			{
				start = ind + 3;
				ind2 = index_of( line, "\n", start );
				if( ind2 > 0 )
				{
					end = ind2 - start;
					substring( line, val, start, end );
					terminal.id = atoi( val );
				}
			}
			else
			{
				ind = index_of( line, "PhoneNumber=", 0 );
				if( ind >= 0 )
				{
					start = ind + 12;
					ind2 = index_of( line, "\n", start );
					if( ind2 > 0 )
					{
						end = ind2 - start;
						substring( line, val, start, end );
						strcpy( terminal.phone_number, val );
					}
				}
			}
		}
		fclose( file );
    }
}

void save_settings()
{
	FILE* f = fopen( SETTINGS_PATH, "w" );
	if( f != NULL )
	{
		fprintf( f, "Id=%d\n", terminal.id );
		fprintf( f, "PhoneNumber=%s\n", terminal.phone_number );
		fflush( f );
		fsync( fileno( f ) );
		fclose( f );
	}
}

void init_log()
{
	FILE *f = fopen( LOGFILE_PATH, "w" );
	if( f != NULL )
		fclose( f );
}

void init()
{
#ifdef __arm__
	char buf[256];
	readlink( "/proc/self/exe", buf, sizeof( buf ) );
	chdir( dirname( buf ) );

	strcpy( intro_file_path, INTRO_KZ_PATH );
#else
	strcpy( intro_file_path, gst_filename_to_uri( INTRO_KZ_PATH, NULL ) );
#endif

	init_log();

	lang = LANG_EN;
	init_locale();
	lang = LANG_RU;

	font_labels = pango_font_description_from_string( "FreeSans 20" );
	font_buttons = pango_font_description_from_string( "FreeSans bold 20" );
	font_instructions = pango_font_description_from_string( "FreeSans bold 22" );

	init_settings();
	init_locale();
	if( !load_transaction() )
		reset_transaction_info();
	else if( transaction.amount > 0 && !transaction.active )
		activate_transaction();

	reset_card_info();
	load_service_cards();
}

void cleanup()
{
	free( servicecards );
}

void quit()
{
	gtk_main_quit();
}

GdkPixbuf* load_image_from_resource( char* resname )
{
	GError* error = NULL;
	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_resource( resname, &error );
	if( !pixbuf )
	{
		fprintf( stderr, "%s\n", error->message );
		g_error_free( error );
	}
	return pixbuf;
}

GdkPixbufAnimation* load_animation_from_resource( char* resname )
{
	GError* error = NULL;
	GdkPixbufAnimation* pixbuf = gdk_pixbuf_animation_new_from_resource( resname, &error );
	if( !pixbuf )
	{
		char err[128];
		sprintf( err, "%s", error->message );
		trace( err );
		g_error_free( error );
	}
	return pixbuf;
}

void set_language( enum APP_LANG language )
{
	lang = language;
	init_locale();
	refresh_form();
}

void start_counter_timer()
{
	if( counter_timer_id == 0 )
	{
		time_counter = 0;
		counter_timer_id = gdk_threads_add_timeout( 1000, counter_timer_callback, NULL );
	}
}

void stop_counter_timer()
{
	if( counter_timer_id > 0 )
	{
		g_source_remove( counter_timer_id );
		counter_timer_id = 0;
	}
}

void reset_counter_timer()
{
	// trace( "Resetting timer" );
	stop_counter_timer();
	start_counter_timer();
}


void save_service_cards()
{
	if( servicecards != NULL && servicecards_count > 0 )
	{
		FILE* file = fopen( SERVICE_CARDS_PATH, "wb" );
		if( file != NULL )
		{
			// trace( "servicecards_count: %d", servicecards_count );
			fwrite( &servicecards_count, sizeof( int ), 1, file );
			for( int i = 0; i < servicecards_count; i++ )
				fwrite( &servicecards[i], sizeof( struct service_card_info ), 1, file );
			fclose( file );
		}
	}
}

void load_service_cards()
{
	FILE* file = fopen( SERVICE_CARDS_PATH, "rb" );
	if( file != NULL )
	{
		fread( &servicecards_count, sizeof( int ), 1, file );
		if( servicecards_count > 0 )
		{
			// trace( "servicecards_count: %d", servicecards_count );
	    	if( servicecards != NULL )
	    		servicecards = realloc( servicecards, servicecards_count * sizeof( struct service_card_info ) );
	    	else
	    		servicecards = malloc( servicecards_count * sizeof( struct service_card_info ) );

			for( int i = 0; i < servicecards_count; i++ )
			{
				fread( &servicecards[i], sizeof( struct service_card_info ), 1, file );
				// trace( "servicecard[%d]: %s %d %s", i, servicecards[i].pan, servicecards[i].type, servicecards[i].pin );
			}
		}
		fclose( file );
	}
}

int get_card_type( const char* pan )
{
	for( int i = 0; i < servicecards_count; i++ )
	{
		if( strcmp( servicecards[i].pan, pan ) == 0 )
			return servicecards[i].type;
	}
	return 0;
}

int get_trials_count( const char* pan )
{
	for( int i = 0; i < servicecards_count; i++ )
	{
		if( strcmp( servicecards[i].pan, pan ) == 0 )
			return servicecards[i].trials;
	}
	return 0;
}

int pin_code_match( const char* pan, const char* pin )
{
	for( int i = 0; i < servicecards_count; i++ )
	{
		if( strcmp( servicecards[i].pan, pan ) == 0 )
		{
			if( servicecards[i].trials < 3 )
			{
				if( strcmp( servicecards[i].pin, pin ) == 0 )
				{
					servicecards[i].trials = 0;
					return 0;
				}
				else
				{
					servicecards[i].trials ++;
					return servicecards[i].trials;
				}
			}
			else
				return servicecards[i].trials;
		}
	}
	return 4;
}

void trace_transaction()
{
	printf( "Id %s\n", transaction.id );
	printf( "Pan %s\n", transaction.pan );
	printf( "Uid %s\n", transaction.uid );
	printf( "Amount %d\n", transaction.amount );
	printf( "SessionAmount %d\n", transaction.session_amount );
	printf( "Cost %d\n", transaction.cost );
	printf( "Type %d\n", transaction.type );
	printf( "FzNumber %s\n", transaction.fz_number );
	printf( "Status %d\n", transaction.status );
	printf( "Active %d\n", transaction.active );
	printf( "ReceiptPrinted %d\n", transaction.receipt_printed );
	char buf[32];
    struct tm* tm_info = localtime( &transaction.beg );
    strftime( buf, 32, "%d.%m.%y %H:%M:%S", tm_info );
	printf( "Beg %s\n", buf );
	tm_info = localtime( &transaction.end );
	strftime( buf, 32, "%d.%m.%y %H:%M:%S", tm_info );
	printf( "End %s\n\n", buf );
}

void save_transaction()
{
	FILE* file = fopen( TRANSACTION_PATH, "wb" );
	if( file != NULL )
	{
		fwrite( &transaction, sizeof( struct transaction_info ), 1, file );
		fclose( file );
	}

	/*
	FILE* f = fopen( TRANSACTION_PATH, "w" );
	if( f != NULL )
	{
		fprintf( f, "Id %s\n", transaction.id );
		fprintf( f, "Pan %s\n", transaction.pan );
		fprintf( f, "Uid %s\n", transaction.uid );
		fprintf( f, "Amount %d\n", transaction.amount );
		fprintf( f, "SessionAmount %d\n", transaction.session_amount );
		fprintf( f, "Cost %d\n", transaction.cost );
		fprintf( f, "Type %d\n", transaction.type );
		fprintf( f, "FzNumber %s\n", transaction.fz_number );
		fprintf( f, "Status %d\n", transaction.status );
		fprintf( f, "Active %d\n", transaction.active );
		fprintf( f, "ReceiptPrinted %d\n", transaction.receipt_printed );
		fprintf( f, "Beg %ld\n", transaction.beg );
		fprintf( f, "End %ld\n", transaction.end );
		fflush( f );
		fsync( fileno( f ) );
		fclose( f );
	}
	*/
}

void log_transaction( int amount )
{
	char path[256], buf[32];
	strcpy( path, TRANSACTION_LOG_DIRECTORY );
    time_t now = time( NULL );
    struct tm* tm_info = localtime( &now );
    strftime( buf, 32, "%Y-%m-%d", tm_info );
    strcat( path, buf );

	FILE* f = fopen( path, "a" );
	if( f != NULL )
	{
		strftime( buf, 32, "%H:%M:%S", tm_info );
		fprintf( f, "%s\t\t%s\t%s\t%s\t%d\t%d\t%d\n", buf, transaction.id, transaction.pan, transaction.uid, amount, transaction.cost, transaction.type );
		fclose( f );
	}
}

gboolean load_transaction()
{
	FILE* file = fopen( TRANSACTION_PATH, "rb" );
	if( file != NULL )
	{
		if( fread( &transaction, sizeof( struct transaction_info ), 1, file ) <= 0 )
			trace( "Failed to load transaction file: %d (%s)", errno, strerror( errno ) );
		fclose( file );
		return TRUE;
	}
	return FALSE;

	/*
	FILE* file = fopen( TRANSACTION_PATH, "r" );
	if( file != NULL )
	{
		char line[512], val[64];
		while( fgets( line, sizeof( line ), file ) )
		{
			if( value_substring( line, "Id ", val ) )
				strcpy( transaction.id, val );
			else if( value_substring( line, "Pan ", val ) )
				strcpy( transaction.pan, val );
			else if( value_substring( line, "Uid ", val ) )
				strcpy( transaction.uid, val );
			else if( value_substring( line, "SessionAmount ", val ) )
				transaction.session_amount = atoi( val );
			else if( value_substring( line, "Amount ", val ) )
				transaction.amount = atoi( val );
			else if( value_substring( line, "Cost ", val ) )
				transaction.cost = atoi( val );
			else if( value_substring( line, "Type ", val ) )
				transaction.type = atoi( val );
			else if( value_substring( line, "FzNumber ", val ) )
				strcpy( transaction.fz_number, val );
			else if( value_substring( line, "Status ", val ) )
				transaction.status = atoi( val );
			else if( value_substring( line, "Active ", val ) )
				transaction.active = atoi( val );
			else if( value_substring( line, "ReceiptPrinted ", val ) )
				transaction.receipt_printed = atoi( val );
			else if( value_substring( line, "Beg ", val ) )
				transaction.beg = atol( val );
			else if( value_substring( line, "End ", val ) )
				transaction.end = atol( val );
		}
		fclose( file );
		return TRUE;
	}
	return FALSE;
	*/
}

void init_transaction( int id, enum TRANSACTION_TYPE type, const char* pan, const char* uid, int amount, int cost )
{
	uuid_t tid;
	char uuid[40];
	uuid_generate( tid );

	transaction.beg = time( NULL );
	transaction.end = transaction.beg;

	if( uuid_parse( uuid, tid ) > -1 )
		sprintf( transaction.id, "%d-%s", id, uuid );
	else
		sprintf( transaction.id, "%d-%ld", id, transaction.beg );

	strcpy( transaction.pan, pan );
	strcpy( transaction.uid, uid );
	transaction.type = type;
	transaction.amount = amount;
	if( transaction.type != TRANSACTION_COLLECT )
		transaction.session_amount += amount;
	transaction.cost = cost;
	transaction.active = FALSE;
	transaction.receipt_printed = FALSE;
	transaction.card_dispensed = FALSE;
	bzero( transaction.fz_number, sizeof( transaction.fz_number ) );
	transaction.status = TRANSACTION_INITIAL;

	save_transaction();
	log_transaction( amount );
}

void append_transaction( int amount )
{
	if( transaction.amount > 0 )
	{
		transaction.end = time( NULL );
		transaction.amount += amount;
		if( transaction.type != TRANSACTION_COLLECT )
			transaction.session_amount += amount;

		save_transaction();
		log_transaction( amount );
	}
	else
		trace( "Error appending transaction!" );
}

void reset_transaction_info()
{
	strcpy( transaction.id, "" );
	strcpy( transaction.uid, "" );
	strcpy( transaction.pan, "" );
	strcpy( transaction.fz_number, "" );
	transaction.amount = 0;
	if( transaction.type == TRANSACTION_COLLECT )
		transaction.session_amount = 0;
	transaction.cost = 0;
	transaction.type = 0;
	transaction.status = (int)TRANSACTION_INITIAL;
	transaction.receipt_printed = FALSE;
	transaction.active = FALSE;
	transaction.card_dispensed = FALSE;
}

void clear_transaction()
{
	reset_transaction_info();
	save_transaction();
}

void close_complete_transaction()
{
	if( transaction.status == (int)TRANSACTION_COMPLETE )
		clear_transaction();
}

void activate_transaction()
{
	transaction.active = TRUE;
	save_transaction();
}

gboolean pending_transaction()
{
	return ( transaction.amount > 0 );
}

gboolean interaction_in_progress()
{
	return ( curr_screen != SCREEN_HOME && curr_screen != SCREEN_OUT_OF_SERVICE && curr_screen != SCREEN_INTRO );
}


void binsprintf( char *s, int val )
{
	char *p;
	unsigned int t;
	p = s;
	t = 0x80000000; // scan 32 bits
	for( ; t > 0; t = t >> 1 )
	{
		if (val & t)
			*p++ = '1';
		else
			*p++ = '0';
	}
	*p = 0;
}

void print_cheque()
{
	print_transaction = transaction;
	print_cheque_requested = TRUE;
}

/*
void draw_cheque( cairo_t* cr, int width, int height )
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"

    gboolean fiscalCheque = ( strlen( print_transaction.fz_number ) > 0 );
    gboolean refill = FALSE;

    int fontSize = 16;
    float leftMargin = 0;
    float fHeight = fontSize * 1.5;
    float yPos = fHeight;

	cairo_set_source_rgb( cr, 1, 1, 1 );
	cairo_rectangle( cr, 0, 0, width, height );
	cairo_fill( cr );
	cairo_set_source_rgb( cr, 0, 0, 0 );
	cairo_select_font_face( cr, "FreeSans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD );
	cairo_set_font_size( cr, fontSize );

    char line[256];
    strcpy( line, _("ChequeOnay") );

	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );
    yPos += fHeight;

    if( !fiscalCheque )
    {
        strcpy( line, _("ChequeSalesReceipt") );
    	cairo_move_to( cr, leftMargin, yPos );
    	cairo_show_text( cr, line );
        yPos += fHeight;
    }

    strcpy( line, _("ChequeAgent") );
	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );
    yPos += fHeight;

    sprintf( line, _("ChequeIIN"), terminal.iin );
	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );
    yPos += fHeight;
    sprintf( line, _("ChequePhoneNumber"), terminal.phone_number );
	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );

    if( fiscalCheque )
    {
        yPos += 2 * fHeight;
        sprintf( line, _("ChequeKKMNumber"), terminal.kkm );
    	cairo_move_to( cr, leftMargin, yPos );
    	cairo_show_text( cr, line );
        yPos += fHeight;
        sprintf( line, _("ChequeKKMRegNumber"), terminal.kkm_reg_number );
    	cairo_move_to( cr, leftMargin, yPos );
    	cairo_show_text( cr, line );

        yPos += 2 * fHeight;
        sprintf( line, _("ChequeFiscNumber"), print_transaction.fz_number );
    	cairo_move_to( cr, leftMargin, yPos );
    	cairo_show_text( cr, line );
    }

    yPos += fHeight;
    strcpy( line, _("ChequeAddress") );
	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );
    yPos += fHeight;

    time_t now = time( NULL );
    struct tm* tm_info = localtime( &now );
    char buf[32];
	strftime( buf, 32, "%d.%m.%y %H:%M:%S", tm_info );
    sprintf( line, _("ChequeDate"), buf );
	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );

    yPos += 2 * fHeight;
    strcpy( line, _("ChequeSupplier") );
	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );
    yPos += fHeight;
    strcpy( line, _("ChequeSupplier2") );
	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );
    yPos += fHeight;
    sprintf( line, _("ChequePhoneNumber"), terminal.phone_number_2 );
	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );

    yPos += 2 * fHeight;
    if( print_transaction.type == TRANSACTION_REFILL )
    {
        sprintf( line, _("ChequeOpRefill"), print_transaction.amount );
        refill = TRUE;
    }
    else if( print_transaction.type == TRANSACTION_BUY_CARD )
    {
        sprintf( line, _("ChequeOpBuyCard"), print_transaction.cost );
        if( print_transaction.amount > print_transaction.cost )
        {
        	cairo_move_to( cr, leftMargin, yPos );
        	cairo_show_text( cr, line );
        	sprintf( line, _("ChequeOpRefill"), print_transaction.amount - print_transaction.cost );
            yPos += fHeight;
            refill = TRUE;
        }
    }
    else if( print_transaction.type == TRANSACTION_BUY_PASS )
    {
        if( print_transaction.amount > print_transaction.cost )
        {
        	cairo_move_to( cr, leftMargin, yPos );
        	cairo_show_text( cr, line );
        	sprintf( line, _("ChequeOpRefill"), print_transaction.amount - print_transaction.cost );
            yPos += fHeight;
            refill = TRUE;
        }
        else if( print_transaction.amount == print_transaction.cost )
        {
            sprintf( line, _("ChequeOpBuyPass"), print_transaction.cost );
        }
        else
        {
            sprintf( line, _("ChequeOpBuyPass"), print_transaction.amount );
            refill = TRUE;
        }
    }
    else if( print_transaction.type == TRANSACTION_COLLECT )
    {
        strcpy( line, _("ChequeOpCollect") );
    	cairo_move_to( cr, leftMargin, yPos );
    	cairo_show_text( cr, line );
    	sprintf( line, _("ChequeAmountExtracted"), print_transaction.amount );
        yPos += fHeight;
    }

	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );
    yPos += fHeight;

    float vat = (print_transaction.amount / 1.12 * 0.12);
    sprintf( line, _("ChequeVAT"), (int)vat );
	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );
	strcpy( line, _("ChequeVATDesc") );
    yPos += fHeight;
	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );
    yPos += fHeight;

    if( print_transaction.type == TRANSACTION_COLLECT )
    {
    	sprintf( line, _("ChequeOpCode"), print_transaction.id );
    	cairo_move_to( cr, leftMargin, yPos );
    	cairo_show_text( cr, line );
    }
    else
    {
        sprintf( line, _("ChequeAccepted"), print_transaction.amount );
    	cairo_move_to( cr, leftMargin, yPos );
    	cairo_show_text( cr, line );
        yPos += fHeight;
        sprintf( line, _("ChequeCommission"), 0 );
    	cairo_move_to( cr, leftMargin, yPos );
    	cairo_show_text( cr, line );
        if( strlen( print_transaction.pan ) > 0 )
        {
            yPos += fHeight;
            sprintf( line, _("ChequeCardNumber"), print_transaction.pan );
        	cairo_move_to( cr, leftMargin, yPos );
        	cairo_show_text( cr, line );
        }
        yPos += fHeight;
        sprintf( line, _("ChequeOpCode"), print_transaction.id );
    	cairo_move_to( cr, leftMargin, yPos );
    	cairo_show_text( cr, line );

        yPos += 2 * fHeight;
        if( fiscalCheque || print_transaction.amount == 0 )
        {
            if( refill )
            {
                strcpy( line, _("ChequeKeepReceiptNote") );
            	cairo_move_to( cr, leftMargin, yPos );
            	cairo_show_text( cr, line );
                strcpy( line, _("ChequeKeepReceiptNote2") );
                yPos += fHeight;
            	cairo_move_to( cr, leftMargin, yPos );
            	cairo_show_text( cr, line );
            }
            else if( print_transaction.amount >= print_transaction.cost || print_transaction.type == TRANSACTION_BUY_PASS )
            {
                strcpy( line, _("ChequeThanksNote") );
            	cairo_move_to( cr, leftMargin, yPos );
            	cairo_show_text( cr, line );
            }
        }
        else
        {
            strcpy( line, _("ChequeNotFiscalNote") );
        	cairo_move_to( cr, leftMargin, yPos );
        	cairo_show_text( cr, line );
            yPos += fHeight;
            strcpy( line, _("ChequeNotFiscalNote2") );
        	cairo_move_to( cr, leftMargin, yPos );
        	cairo_show_text( cr, line );
            yPos += fHeight;
            strcpy( line, _("ChequeNotFiscalNote3") );
        	cairo_move_to( cr, leftMargin, yPos );
        	cairo_show_text( cr, line );
            yPos += fHeight;
            strcpy( line, _("ChequeNotFiscalNote4") );
        	cairo_move_to( cr, leftMargin, yPos );
        	cairo_show_text( cr, line );
            yPos += fHeight;
            strcpy( line, _("ChequeNotFiscalNote5") );
        	cairo_move_to( cr, leftMargin, yPos );
        	cairo_show_text( cr, line );
        }
    }

    int m = 23 - (int)( yPos / fHeight );
    if( m > 0 )
        yPos += m * fHeight;
    else
        yPos += fHeight;

    strcpy( line, _("ChequeRoundTheClock") );
	cairo_move_to( cr, leftMargin, yPos );
	cairo_show_text( cr, line );

#pragma GCC diagnostic pop
}
*/


typedef struct ConvLetter {
        char win1251;
        int  unicode;
} Letter;

// ә, ғ, қ, ң, ө, ұ, ү, h, і, Ә, Ғ, Қ, Ң, Ө, Ұ, Ү, Һ, І
static Letter g_letters[] = {
	{0x80, 0x04B0}, // KAZAKH CAPITAL LETTER Ұ
	{0x81, 0x0492}, // KAZAKH CAPITAL LETTER Ғ
	{0x82, 0x201A}, // SINGLE LOW-9 QUOTATION MARK
	// {0x83, 0x0453}, // CYRILLIC SMALL LETTER GJE
	{0x83, 0x0493}, // KAZAKH SMALL LETTER ғ
	{0x84, 0x201E}, // DOUBLE LOW-9 QUOTATION MARK
	{0x85, 0x2026}, // HORIZONTAL ELLIPSIS
	{0x86, 0x2020}, // DAGGER
	{0x87, 0x2021}, // DOUBLE DAGGER
	{0x88, 0x20AC}, // EURO SIGN
	{0x89, 0x2030}, // PER MILLE SIGN
	// {0x8A, 0x0409}, // CYRILLIC CAPITAL LETTER LJE
	{0x8A, 0x04E8}, // KAZAKH CAPITAL LETTER Ө
	{0x8B, 0x2039}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
	// {0x8C, 0x040A}, // CYRILLIC CAPITAL LETTER NJE
	// {0x8D, 0x040C}, // CYRILLIC CAPITAL LETTER KJE
	// {0x8E, 0x040B}, // CYRILLIC CAPITAL LETTER TSHE
	// {0x8F, 0x040F}, // CYRILLIC CAPITAL LETTER DZHE
	// {0x90, 0x0452}, // CYRILLIC SMALL LETTER DJE
	{0x8C, 0x04A2}, // KAZAKH CAPITAL LETTER Ң
	{0x8D, 0x049A}, // KAZAKH CAPITAL LETTER Қ
	{0x8E, 0x04BA}, // KAZAKH CAPITAL LETTER Һ
	{0x8F, 0x04AE}, // KAZAKH CAPITAL LETTER Ү
	{0x90, 0x04B1}, // KAZAKH SMALL LETTER ұ
	{0x91, 0x2018}, // LEFT SINGLE QUOTATION MARK
	{0x92, 0x2019}, // RIGHT SINGLE QUOTATION MARK
	{0x93, 0x201C}, // LEFT DOUBLE QUOTATION MARK
	{0x94, 0x201D}, // RIGHT DOUBLE QUOTATION MARK
	{0x95, 0x2022}, // BULLET
	{0x96, 0x2013}, // EN DASH
	{0x97, 0x2014}, // EM DASH
	{0x99, 0x2122}, // TRADE MARK SIGN
	// {0x9A, 0x0459}, // CYRILLIC SMALL LETTER LJE
	{0x9A, 0x04E9}, // KAZAKH SMALL LETTER ө
	{0x9B, 0x203A}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
	// {0x9C, 0x045A}, // CYRILLIC SMALL LETTER NJE
	// {0x9D, 0x045C}, // CYRILLIC SMALL LETTER KJE
	// {0x9E, 0x045B}, // CYRILLIC SMALL LETTER TSHE
	// {0x9F, 0x045F}, // CYRILLIC SMALL LETTER DZHE
	{0x9C, 0x04A3}, // KAZAKH SMALL LETTER ң
	{0x9D, 0x049B}, // KAZAKH SMALL LETTER қ
	{0x9E, 0x04BB}, // KAZAKH SMALL LETTER h
	{0x9F, 0x04AF}, // KAZAKH SMALL LETTER ү
	{0xA0, 0x00A0}, // NO-BREAK SPACE
	{0xA1, 0x040E}, // CYRILLIC CAPITAL LETTER SHORT U
	{0xA2, 0x045E}, // CYRILLIC SMALL LETTER SHORT U
	{0xA3, 0x0408}, // CYRILLIC CAPITAL LETTER JE
	{0xA4, 0x00A4}, // CURRENCY SIGN
	{0xA5, 0x0490}, // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
	{0xA6, 0x00A6}, // BROKEN BAR
	{0xA7, 0x00A7}, // SECTION SIGN
	{0xA8, 0x0401}, // CYRILLIC CAPITAL LETTER IO
	{0xA9, 0x00A9}, // COPYRIGHT SIGN
	{0xAA, 0x0404}, // CYRILLIC CAPITAL LETTER UKRAINIAN IE
	{0xAB, 0x00AB}, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
	{0xAC, 0x00AC}, // NOT SIGN
	{0xAD, 0x00AD}, // SOFT HYPHEN
	{0xAE, 0x00AE}, // REGISTERED SIGN
	{0xAF, 0x0407}, // CYRILLIC CAPITAL LETTER YI
	{0xB0, 0x00B0}, // DEGREE SIGN
	{0xB1, 0x00B1}, // PLUS-MINUS SIGN
	// {0xB2, 0x0406}, // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
	// {0xB3, 0x0456}, // CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
	{0xB2, 0x0406}, // KAZAKH CAPITAL LETTER І
	{0xB3, 0x0456}, // KAZAKH SMALL LETTER і
	{0xB4, 0x0491}, // CYRILLIC SMALL LETTER GHE WITH UPTURN
	{0xB5, 0x00B5}, // MICRO SIGN
	{0xB6, 0x00B6}, // PILCROW SIGN
	{0xB7, 0x00B7}, // MIDDLE DOT
	{0xB8, 0x0451}, // CYRILLIC SMALL LETTER IO
	{0xB9, 0x2116}, // NUMERO SIGN
	{0xBA, 0x0454}, // CYRILLIC SMALL LETTER UKRAINIAN IE
	{0xBB, 0x00BB}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
	{0xBC, 0x0458}, // CYRILLIC SMALL LETTER JE
	// {0xBD, 0x0405}, // CYRILLIC CAPITAL LETTER DZE
	// {0xBE, 0x0455}, // CYRILLIC SMALL LETTER DZE
	{0xBD, 0x04D8}, // KAZAKH CAPITAL LETTER Ә
	{0xBE, 0x04D9}, // KAZAKH SMALL LETTER ә
	{0xBF, 0x0457}  // CYRILLIC SMALL LETTER YI
};

int utf8_to_cp1251( const char* utf8, char* windows1251, size_t n, int append_newlines )
{
	int i = 0;
	int j = 0;
	for( ; i < (int)n && utf8[i] != 0; ++i )
	{
		char prefix = utf8[i];
		char suffix = utf8[i+1];

		// trace( "Pref: %d  Suff: %d", prefix, suffix );

		if( (prefix & 0x80) == 0 )
		{
			windows1251[j] = (char)prefix;
			++j;
		}
		else if( prefix == 226 && suffix == 132 )
		{
			windows1251[j] = (char)0xB9;
			++i;
			goto NEXT_LETTER;
		}
		else if( (~prefix) & 0x20 )
		{
			int first5bit = prefix & 0x1F;
			first5bit <<= 6;
			int sec6bit = suffix & 0x3F;
			int unicode_char = first5bit + sec6bit;

			if( unicode_char >= 0x410 && unicode_char <= 0x44F )
			{
				windows1251[j] = (char)(unicode_char - 0x350);
			}
			else if( unicode_char >= 0x80 && unicode_char <= 0xFF )
			{
				windows1251[j] = (char)(unicode_char);
			}
			else if( unicode_char >= 0x402 && unicode_char <= 0x403 )
			{
				windows1251[j] = (char)(unicode_char - 0x382);
			}
			else
			{
				int count = sizeof( g_letters ) / sizeof( Letter );
				for( int k = 0; k < count; ++k )
				{
					if( unicode_char == g_letters[k].unicode )
					{
						windows1251[j] = g_letters[k].win1251;
						goto NEXT_LETTER;
					}
				}
				// can't convert this char
trace( "F: %d at %d", unicode_char, j );
				return 0;
			}
NEXT_LETTER:

// trace( "U: %d", unicode_char );
			++i;
			++j;
		}
		else
		{
// trace( "Wrong prefixes: %d %d", prefix, suffix );
			// can't convert this chars
			return 0;
		}
	}

	for( int c = 0; c < append_newlines; c++ )
	{
		windows1251[j++] = '\r';
		windows1251[j++] = '\n';
	}

	windows1251[j] = 0;
	return j;
}

long timediff( clock_t t1, clock_t t2 )
{
    long elapsed;
    elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
    return elapsed;
}

void print_cheque_sync( int fd )
{
	// clock_t t1 = clock();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"

	int len = 0;
	int TEXT_SIZE = 4096;
	int LINE_SIZE = 1024;
	char text[TEXT_SIZE];
    char line[LINE_SIZE];
    char buff[LINE_SIZE];

    /*
    bzero( line, LINE_SIZE );
    len = utf8_to_cp1251( "№", line, LINE_SIZE, 1 ); // _("ChequeVATDesc")
trace( "Len: %d", len );
return;
	*/

    gboolean fiscalCheque = ( strlen( print_transaction.fz_number ) > 0 );
    gboolean refill = FALSE;

    len += utf8_to_cp1251( _("ChequeOnay"), line, LINE_SIZE, 1 );
    strcpy( text, line );

    if( !fiscalCheque )
    {
        len += utf8_to_cp1251( _("ChequeSalesReceipt"), line, LINE_SIZE, 1 );
        strcat( text, line );
    }

    len += utf8_to_cp1251( _("ChequeAgent"), line, LINE_SIZE, 1 );
    strcat( text, line );

    sprintf( buff, _("ChequeIIN"), terminal.iin );
    len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
    strcat( text, line );

    sprintf( buff, _("ChequePhoneNumber"), terminal.phone_number );
    len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
    strcat( text, line );

    if( fiscalCheque )
    {
        strcat( text, "\r\n" );
        len += 2;
        sprintf( buff, _("ChequeKKMNumber"), terminal.kkm );
        len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
        strcat( text, line );
        sprintf( buff, _("ChequeKKMRegNumber"), terminal.kkm_reg_number );
        len += utf8_to_cp1251( buff, line, LINE_SIZE, 2 );
        strcat( text, line );
        sprintf( buff, _("ChequeFiscNumber"), print_transaction.fz_number );
        len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
        strcat( text, line );
    }

    len += utf8_to_cp1251( _("ChequeAddress"), line, LINE_SIZE, 1 );
    strcat( text, line );

    time_t now = time( NULL );
    struct tm* tm_info = localtime( &now );
    char buf[32];
	strftime( buf, 32, "%d.%m.%y %H:%M:%S", tm_info );
    sprintf( buff, _("ChequeDate"), buf );
    len += utf8_to_cp1251( buff, line, LINE_SIZE, 2 );
    strcat( text, line );

    len += utf8_to_cp1251( _("ChequeSupplier"), line, LINE_SIZE, 1 );
    strcat( text, line );

    sprintf( buff, _("ChequePhoneNumber"), terminal.phone_number_2 );
    len += utf8_to_cp1251( buff, line, LINE_SIZE, 2 );
    strcat( text, line );

    if( print_transaction.type == TRANSACTION_REFILL )
    {
        sprintf( buff, _("ChequeOpRefill"), print_transaction.amount );
        len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
        refill = TRUE;
    }
    else if( print_transaction.type == TRANSACTION_BUY_CARD )
    {
        sprintf( buff, _("ChequeOpBuyCard"), print_transaction.cost );
        len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
        if( print_transaction.amount > print_transaction.cost )
        {
        	strcat( text, line );
        	sprintf( buff, _("ChequeOpRefill"), print_transaction.amount - print_transaction.cost );
        	len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
            refill = TRUE;
        }
    }
    else if( print_transaction.type == TRANSACTION_BUY_PASS )
    {
        if( print_transaction.amount > print_transaction.cost )
        {
        	strcat( text, line );
        	sprintf( buff, _("ChequeOpRefill"), print_transaction.amount - print_transaction.cost );
        	len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
            refill = TRUE;
        }
        else if( print_transaction.amount == print_transaction.cost )
        {
            sprintf( buff, _("ChequeOpBuyPass"), print_transaction.cost );
            len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
        }
        else
        {
            sprintf( buff, _("ChequeOpBuyPass"), print_transaction.amount );
            len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
            refill = TRUE;
        }
    }
    else if( print_transaction.type == TRANSACTION_COLLECT )
    {
        len += utf8_to_cp1251( _("ChequeOpCollect"), line, LINE_SIZE, 1 );
        strcat( text, line );
    	sprintf( buff, _("ChequeAmountExtracted"), print_transaction.amount );
    	len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
    }

    strcat( text, line );

    float vat = (print_transaction.amount / 1.12 * 0.12);
    sprintf( buff, _("ChequeVAT"), (int)vat );
    len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
    strcat( text, line );

    len += utf8_to_cp1251( _("ChequeVATDesc"), line, LINE_SIZE, 1 );
    strcat( text, line );

    if( print_transaction.type == TRANSACTION_COLLECT )
    {
    	sprintf( buff, _("ChequeOpCode"), print_transaction.id );
        len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
        strcat( text, line );
    }
    else
    {
        sprintf( buff, _("ChequeAccepted"), print_transaction.amount );
        len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
        strcat( text, line );
        sprintf( buff, _("ChequeCommission"), 0 );
        len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
        strcat( text, line );
        if( strlen( print_transaction.pan ) > 0 )
        {
            sprintf( buff, _("ChequeCardNumber"), print_transaction.pan );
            len += utf8_to_cp1251( buff, line, LINE_SIZE, 1 );
            strcat( text, line );
        }
        sprintf( buff, _("ChequeOpCode"), print_transaction.id );
        len += utf8_to_cp1251( buff, line, LINE_SIZE, 2 );
        strcat( text, line );

        if( fiscalCheque || print_transaction.amount == 0 )
        {
            if( refill )
            {
                len += utf8_to_cp1251( _("ChequeKeepReceiptNote"), line, LINE_SIZE, 1 );
                strcat( text, line );
            }
            else if( print_transaction.amount >= print_transaction.cost || print_transaction.type == TRANSACTION_BUY_PASS )
            {
                len += utf8_to_cp1251( _("ChequeThanksNote"), line, LINE_SIZE, 1 );
                strcat( text, line );
            }
        }
        else
        {
            len += utf8_to_cp1251( _("ChequeNotFiscalNote"), line, LINE_SIZE, 1 );
            strcat( text, line );
        }
    }

    strcat( text, "\r\n" );
    len += 2;
    len += utf8_to_cp1251( _("ChequeRoundTheClock"), line, LINE_SIZE, 1 );
    strcat( text, line );

	if( len > TEXT_SIZE )
		len = TEXT_SIZE;

	char char_set[] = { 0x1b, 0x52, 0x0f };
	char print_lf[] = { 0x1b, 0x64, 0x0a };

	write( fd, char_set, sizeof( char_set ) );

    write( fd, text, len );

	write( fd, print_lf, sizeof( print_lf ) );

#pragma GCC diagnostic pop

	// clock_t t2 = clock();
    // long elapsed = timediff( t1, t2 );
    // trace( "elapsed: %ld ms\n", elapsed );

/*
# print cheque as an image
	// int delay_microseconds = 200 * 1000;
	unsigned char xL = 64, xH = 0, yL = 192, yH = 2;
	unsigned int width = (xL + xH * 256) * 8, height = (yL + yH * 256);
	// unsigned int width = 512, height = 704;

    cairo_surface_t* surface = cairo_image_surface_create( CAIRO_FORMAT_RGB24, width, height ); // CAIRO_FORMAT_A1 CAIRO_FORMAT_ARGB32
    cairo_t* cr = cairo_create( surface );

    draw_cheque( cr, width, height );

    int stride = cairo_image_surface_get_stride( surface );
    int stride_times_height = stride * height;

    unsigned char* ptr = (unsigned char*)malloc( stride_times_height );
    memcpy( ptr, cairo_image_surface_get_data( surface ), stride_times_height );

    int pix_size = width * height / 8, data_size = pix_size + 8;
	char pix_data[data_size];
	bzero( pix_data, data_size );

	pix_data[0] = 0x1D;
	pix_data[1] = 0x76;
	pix_data[2] = 0x30;
	pix_data[3] = 0x00;
	pix_data[4] = xL;
	pix_data[5] = xH;
	pix_data[6] = yL;
	pix_data[7] = yH;

    uint32_t ind = 0, off = 0;
    for( int j = 0; j < height; j++ )
    {
		uint32_t* row = (uint32_t *) &ptr[j * stride];
		for( int i = 0; i < width; i++ )
		{
			uint32_t px = row[i];
			if( px < 0xff808080 )
			{
				off = 7 - i % 8;
				ind = 8 + (i + j * width) / 8;
				pix_data[ind] = ( pix_data[ind] | ( 1 << off ) );
			}
		}
    }

	int written = 0, i = 0;
	do
	{
	    written = write( fd, &pix_data[i], data_size );
	    i += written;
	    // usleep( delay_microseconds );
	}
	while( written > 0 && i < data_size );

	if( written < 0 )
		trace( "Error writing to printer: %d (%s)", errno, strerror( errno ) );

    free( ptr );
    cairo_surface_destroy( surface );
    cairo_destroy( cr );
*/

/*
# create bmp-file
    int offset = 14 + 40 + 8, fileSize = offset + pix_size;
    char val[4];
    FILE* fp = fopen( "cheque.bmp", "wb" );
    if( fp != NULL )
    {
    	// file header
    	fputc( 'B', fp );
    	fputc( 'M', fp );
    	int_to_bytes( fileSize, val );
    	// printf( "File size: %d\n", fileSize );
    	fwrite( val, 1, 4, fp );   	// file size in bytes
    	bzero( val, 4 );
    	fwrite( val, 1, 4, fp );   	// reserved
    	int_to_bytes( offset, val );
    	fwrite( val, 1, 4, fp );   	// offset to pixel data

    	// image header
    	int_to_bytes( 40, val );
    	fwrite( val, 1, 4, fp );   	// header size
    	int_to_bytes( width, val );
    	fwrite( val, 1, 4, fp );	// image width in pixels
    	int_to_bytes( height, val );
    	fwrite( val, 1, 4, fp );	// image height in pixels
    	short_to_bytes( 1, val );	// planes
    	fwrite( val, 1, 2, fp );
    	short_to_bytes( 1, val );	// bits per pixel
    	fwrite( val, 1, 2, fp );
    	bzero( val, 4 );
    	fwrite( val, 1, 4, fp );	// compression
    	fwrite( val, 1, 4, fp );	// image size - may be 0 for uncompressed images
    	fwrite( val, 1, 4, fp );	// preferred horizontal resolution in pixels per meter. It's only recommendation and can be 0
    	fwrite( val, 1, 4, fp );	// preferred vertical resolution in pixels per meter
    	fwrite( val, 1, 4, fp );	// number of color map entries. May be 0
    	fwrite( val, 1, 4, fp );	// number of significant colors. If 0 then all colors in the color table are considered important

        // color table: four bytes for each color
        // black for 0-index
    	fwrite( val, 1, 4, fp );
    	// white for 1-index
    	fputc( 0xff, fp );
    	fputc( 0xff, fp );
    	fputc( 0xff, fp );
    	fputc( 0x00, fp );

        char bmp_data[pixSize];
        for( int i = 0; i < pix_size; i++ )
        	bmp_data[i] = 0xff;

        uint32_t ind = 0, off = 0, ind2 = 0;
        for( int j = 0; j < height; j++ )
        {
			uint32_t* row = (uint32_t *) &ptr[j * stride];
			for( int i = 0; i < width; i++ )
			{
				uint32_t px = row[i];
				if( px < 0xff808080 )
				{
					ind = (i + ( height - 1 - j ) * width) / 8;
					off = 7 - i % 8;
					if( ind < pixSize )
						bmp_data[ind] = ( bmp_data[ind] & ~( 1 << off ) );

					ind2 = 8 + (i + j * width) / 8;
					pix_data[ind2] = ( pix_data[ind2] | ( 1 << off ) );
				}
			}
        }
        fwrite( bmp_data, 1, pix_size, fp );
    	fclose( fp );
    }

    free( ptr );
    cairo_surface_destroy( surface );
    cairo_destroy( cr );
*/

/*
# use manifacturer's library function calls (buggy)
#ifdef __arm__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration" // no include file for printing SDK
    //printf( "print: 0 " );
	SetDevname( 1, "/dev/ttyUSB0", 115200 );
	//printf( " 1" );
	SetInit();
	//printf( " 2" );
	usleep( delay_microseconds );
	SetClean();
	//printf( " 3" );
	usleep( delay_microseconds );
	//SetLeftmargin( 0 );
	//printf( " 4" );
	//PrintDiskbmpfile( "cheque.bmp" );
	//printf( " 5" );
	//PrintCutpaper( 0 );
	//printf( " 6" );
	SetClose();
	//printf( " !\n" );

#pragma GCC diagnostic pop
#endif
*/
}


gboolean operating()
{
	return (terminal.connected && !pending_transaction() && printer_status == PRINTER_OK );
}




void enable_cash_retrieval( gboolean enable )
{
	cash_retrieval_enabled = enable;
	// trace( "Cash retrieval enabled: %d", enable );
}

void deposit_amount( int amount )
{
    // this.Session.Add( amount );

    if( transaction.amount > 0 )
        append_transaction( amount );
    else
    {
        switch( curr_state )
        {
            case STATE_BUY_CARD:

            	trace( "Card_price: %d", (int)terminal.card_price );

                init_transaction( terminal.id, TRANSACTION_BUY_CARD, card.pan, card.uid, amount, (int)terminal.card_price );
                break;

            case STATE_BUY_PASS:
				init_transaction( terminal.id, TRANSACTION_BUY_PASS, card.pan, card.uid, amount, (int)( card.pass_price + card.ride_price ) );
                break;

            case STATE_CHECK_REFILL:
            	init_transaction( terminal.id, TRANSACTION_REFILL, card.pan, card.uid, amount, 0 );
                break;

            case STATE_COLLECT:
            	trace( "deposit_amount called at STATE_COLLECT" );
                break;

            case STATE_IDLE:
            	trace( "deposit_amount called at STATE_IDLE" );
            	break;

            case STATE_OUT_OF_SERVICE:
            	trace( "deposit_amount called at STATE_OUT_OF_SERVICE" );
            	break;

            case STATE_MAINTENANCE:
            	trace( "deposit_amount called at STATE_MAINTENANCE" );
            	break;
        }
    }

    if( curr_state == STATE_BUY_CARD )
	{
		if( transaction.amount >= terminal.card_price )
			display_form( SCREEN_CARD_SUFFICIENT_AMOUNT );
		else
			display_form( SCREEN_CARD_PARTIAL_AMOUNT );
	}
    else if( curr_state == STATE_BUY_PASS )
    {
		if( ( transaction.amount + card.balance ) >= ( card.pass_price + card.ride_price ) )
			display_form( SCREEN_PASS_SUFFICIENT_AMOUNT );
		else
			display_form( SCREEN_PASS_PARTIAL_AMOUNT );
    }
    else if( curr_state == STATE_CHECK_REFILL )
    {
        display_form( SCREEN_BALANCE_REFILL );
    }
}


gboolean card_contains_valid_pass( time_t date )
{
	if( card.pass_beg > 0 && card.pass_end > 0 )
	{
		struct tm* tm_info_dt = localtime( &date );
		struct tm* tm_info = localtime( &card.pass_beg );
		if( tm_info_dt->tm_mday >= tm_info->tm_mday && tm_info_dt->tm_mon >= tm_info->tm_mon && tm_info_dt->tm_year >= tm_info->tm_year )
		{
			tm_info = localtime( &card.pass_end );
			if( tm_info_dt->tm_mday <= tm_info->tm_mday && tm_info_dt->tm_mon <= tm_info->tm_mon && tm_info_dt->tm_year <= tm_info->tm_year )
				return TRUE;
		}
	}
	return FALSE;
}

void clear_state()
{
	enable_cash_retrieval( FALSE );
	reset_card_info();
	close_complete_transaction();
	if( curr_state != STATE_OUT_OF_SERVICE )
		curr_state = STATE_IDLE;
}




void card_dispensed()
{
	trace( "Card dispensed!" );
}

void dispense_card()
{
	dispense_attempts = 0;
	card_dispense_requested = TRUE;
}

gboolean dispenser_is_ready()
{
	return ( dispenser_status != DISPENSER_NOT_READY && dispenser_status != DISPENSER_DISPENSING_ERROR && dispenser_status != DISPENSER_EXECUTION_ERROR && terminal.card_price > 0 );
}



void reset_card_info()
{
	strcpy( card.uid, "" );
	strcpy( card.pan, "" );
	card.balance = 0.0f;
	card.ride_price = 0.0f;
	card.pass_price = 0.0f;
	card.status = CARD_NOT_FOUND;
	card.initialized = FALSE;
	card.active = FALSE;
	card.type = 0;
	card.pass_active = FALSE;
	card.pass_beg = 0;
	card.pass_end = 0;
}

void card_activated()
{
	if( card.type == 0 )
	{
		if( curr_state == STATE_IDLE )
			curr_state = STATE_CHECK_REFILL;

		if( transaction.amount > 0 && strlen( transaction.pan ) == 0 )
		{
			strcpy( transaction.pan, card.pan );
			save_transaction();
		}

		card.active = TRUE;
	}
	else if( card.type == 1 )
	{
		curr_state = STATE_COLLECT;
	}
	else if( card.type == 2 )
	{

	}
}

// -------------- app -------------------------------------------------------------------<


// -------------- video ----------------------------------------------------------------->

void start_playback()
{
	// trace( "Starting playback..." );
	gst_element_set_state( pipeline, GST_STATE_READY );

#ifdef __arm__
	g_object_set( G_OBJECT( source ), "location", intro_file_path, NULL );

	if( strstr( intro_file_path, "all_kaz.flv" ) != NULL )
		strcpy( intro_file_path, INTRO_RU_PATH );
	else
		strcpy( intro_file_path, INTRO_KZ_PATH );
#else
	g_object_set( G_OBJECT( pipeline ), "uri", intro_file_path, NULL );

	if( strstr( intro_file_path, "all_kaz.avi" ) != NULL )
		strcpy( intro_file_path, gst_filename_to_uri( INTRO_RU_PATH, NULL ) );
	else
		strcpy( intro_file_path, gst_filename_to_uri( INTRO_KZ_PATH, NULL ) );
#endif

	freopen( "/dev/null","a", stdout );
	gst_element_set_state( pipeline, GST_STATE_PLAYING );
	usleep( 200000 );
	freopen( "/dev/tty", "a", stdout );
}

void stop_playback()
{
	gst_element_set_state( pipeline, GST_STATE_NULL );
	times_intro_played = 0;
}

static void video_widget_realize_cb( GtkWidget* widget, gpointer data )
{
#ifdef GDK_WINDOWING_X11
	video_window_handle = GDK_WINDOW_XID( gtk_widget_get_window( widget ) );
#endif
}

static GstBusSyncReply bus_sync_handler( GstBus* bus, GstMessage* message, gpointer user_data )
{
	// ignore anything but 'prepare-window-handle' element messages
	if( !gst_is_video_overlay_prepare_window_handle_message( message ) )
		return GST_BUS_PASS;

	if( video_window_handle != 0 )
    {
		// GST_MESSAGE_SRC (message) will be the video sink element
		GstVideoOverlay* overlay = GST_VIDEO_OVERLAY( GST_MESSAGE_SRC( message ) );
		gst_video_overlay_set_window_handle( overlay, video_window_handle );
		return GST_BUS_PASS;
    }
	else
    {
		trace( "Should have obtained video_window_handle by now!" );
		return GST_BUS_DROP;
    }
}

/* Process messages from GStreamer */
static gboolean handle_message( GstBus* bus, GstMessage* msg /*, CustomData* data*/ )
{
	// GstState old_state, new_state, pending_state;

	switch( GST_MESSAGE_TYPE( msg ) )
	{
		case GST_MESSAGE_ERROR:
			trace( "Error during playback" );

			GError* err;
			gchar* debug_info;
			gst_message_parse_error( msg, &err, &debug_info );
			trace( "Error received from element %s: %s", GST_OBJECT_NAME( msg->src ), err->message );
			trace( "Debugging information: %s", debug_info ? debug_info : "none" );
			g_clear_error( &err );
			g_free( debug_info );

            stop_playback();
            // display_form( SCREEN_HOME );
            quit();
			break;

		case GST_MESSAGE_EOS:

			// trace( "End-Of-Stream reached" );
            times_intro_played ++;
            if( times_intro_played < 2 )
                start_playback();
            else
            {
                stop_playback();
                display_form( SCREEN_HOME );
            }
			break;

			/*
		case GST_MESSAGE_STATE_CHANGED:
			// trace( "Playback state changed" );


			gst_message_parse_state_changed( msg, &old_state, &new_state, &pending_state );
			trace( "State: %d", new_state );
			if( new_state == GST_STATE_PLAYING )
			{

			}

			break;
			*/

		default:
			break;
	}

	// We want to keep receiving messages
	return TRUE;
}

#ifdef __arm__
static void on_pad_added( GstElement *element, GstPad* pad, gpointer data )
{
	GstElement* decoder = (GstElement*) data;
	// trace( "Dynamic pad created, linking demuxer/decoder\n" );
	GstPad* sinkpad = gst_element_get_static_pad( decoder, "sink" );
	gst_pad_link( pad, sinkpad );
	gst_object_unref( sinkpad );
}
#endif



void init_video()
{
	// https://gstreamer.freedesktop.org/documentation/tutorials/playback/playbin-usage.html
#ifdef __arm__
	// strcpy( intro_file_path, INTRO_KZ_PATH );
	pipeline = gst_pipeline_new( "flv-video-player" );

	source   = gst_element_factory_make( "filesrc", "file-source" );
	GstElement* decoder = gst_element_factory_make( "decodebin", "decoder" );
	GstElement* sink    = gst_element_factory_make( "imxg2dvideosink", "video-output" );

	if( !pipeline || !source || !decoder || !sink )
	{
	    trace( "Not all pipeline elements could be created\n" );
	    return;
	}

	gst_bin_add_many( GST_BIN( pipeline ), source, decoder, sink, NULL );
	gst_element_link_pads( source, "src", decoder, "sink" );
	g_signal_connect( decoder, "pad-added", G_CALLBACK( on_pad_added ), sink );
#else
	// strcpy( intro_file_path, gst_filename_to_uri( INTRO_KZ_PATH, NULL ) );
	pipeline = gst_element_factory_make( "playbin", "play" );
#endif

	bus = gst_pipeline_get_bus( GST_PIPELINE( pipeline ) );
	gst_bus_set_sync_handler( bus, (GstBusSyncHandler)bus_sync_handler, NULL, NULL );
	gst_bus_add_watch( bus, (GstBusFunc)handle_message, NULL );

	// trace( intro_file_path );
}

void free_video()
{
	gst_object_unref( bus );
	gst_element_set_state( pipeline, GST_STATE_NULL );
	gst_object_unref( pipeline );
}
// -------------- video -----------------------------------------------------------------<


// -------------- callbacks ------------------------------------------------------------->

gboolean touch_instruction_callback( GtkWidget* widget, GdkEventButton* event )
{

	// dispense_card();

	return TRUE;




	if( cash_retrieval_enabled )
	{
		if( curr_screen == SCREEN_PASS_FIRST || curr_screen == SCREEN_PASS_PARTIAL_AMOUNT )
			deposit_amount( 500 );
		else
			deposit_amount( 100 );
	}
	else if( curr_screen == SCREEN_HOME || curr_screen == SCREEN_PASS_INTRO || curr_screen == SCREEN_CARD_SCAN )
	{
		strcpy( card.pan, "9643108503301731701" );
		printf( "pan: %s\n", card.pan );
		card.initialized = TRUE;

		display_form( SCREEN_LOADING );
		card_activated();
	}

	return TRUE;
}

gboolean touch_intro_callback( GtkWidget* widget, GdkEventButton* event )
{
	stop_playback();
	if( operating() )
		display_form( SCREEN_HOME );
	else
	{
		display_form( SCREEN_OUT_OF_SERVICE );
	}
	return TRUE;
}

void back_callback( GtkWidget* widget, gpointer data )
{
	if( curr_screen == SCREEN_HOME )
	{
		stop_counter_timer();
		if( stacker_status == STACK_EMPTY || !dispenser_is_ready() )
		{
			display_form( SCREEN_NO_CARDS );
		}
		else
		{
			curr_state = STATE_BUY_CARD;
			display_form( SCREEN_CARD_INTRO );
		}
	}
	else if( curr_screen == SCREEN_MORE_TIME )
	{
		if( curr_state == STATE_BUY_PASS )
		{
			if( transaction.amount > 0 )
			{
				if( ( transaction.amount + card.balance ) < ( card.pass_price + card.ride_price ) )
					display_form( SCREEN_PASS_INSUFFICIENT_AMOUNT );
				else
				{
					activate_transaction();
					display_form( SCREEN_LOADING );
				}
			}
			else
				display_form( SCREEN_HOME );
		}
		else if( curr_state == STATE_BUY_CARD )
		{
			if( transaction.amount > 0 )
			{
				// if( transaction.amount >= transaction.cost )
				// 	  dispense_card();
				activate_transaction();
				display_form( SCREEN_LOADING );
			}
			else
				display_form( SCREEN_HOME );
		}
		else
			display_form( SCREEN_HOME );
	}
	else
	{
		display_form( SCREEN_HOME );
	}
}

void mid_callback( GtkWidget* widget, gpointer data )
{
    display_form( SCREEN_MANUAL_INPUT );
}

void next_callback( GtkWidget* widget, gpointer data )
{
    switch( curr_screen )
    {
        case SCREEN_HOME:
            curr_state = STATE_BUY_PASS;
            display_form( SCREEN_PASS_INTRO );
            break;

        case SCREEN_CARD_PRE_SCAN:
			display_form( SCREEN_CARD_SCAN );
			dispense_card();
        	break;

        case SCREEN_CARD_SUFFICIENT_AMOUNT:

        	enable_cash_retrieval( FALSE );
        	if( transaction.amount == terminal.card_price )
        	{
				display_form( SCREEN_CARD_SCAN );
				dispense_card();
        	}
        	else if( transaction.amount > terminal.card_price )
        	{
        		display_form( SCREEN_CARD_PRE_SCAN );
        	}
            break;

        case SCREEN_PASS_SUFFICIENT_AMOUNT:

            if( transaction.amount == 0 )
                init_transaction( terminal.id, TRANSACTION_BUY_PASS, card.pan, card.uid, 0, (int)( card.pass_price + card.ride_price ) );
            activate_transaction();
            enable_cash_retrieval( FALSE );
            display_form( SCREEN_LOADING );
            break;

        case SCREEN_BALANCE_REFILL:
            activate_transaction();
            enable_cash_retrieval( FALSE );
            display_form( SCREEN_LOADING );
            break;

        case SCREEN_MORE_TIME:

        	if( last_screen == SCREEN_MANUAL_INPUT )
        	{
        		reset_counter_timer();
        		reload_form( last_screen, TRUE );
        	}
        	else if( last_screen == SCREEN_MORE_TIME )
    			display_more_time_form( COUNTDOWN_SECONDS - time_counter );
        	else
        		display_form( last_screen );

            // if( last_screen == SCREEN_PASS_PARTIAL_AMOUNT || last_screen == SCREEN_PASS_SUFFICIENT_AMOUNT || last_screen == SCREEN_CARD_INTRO || last_screen == SCREEN_CARD_PARTIAL_AMOUNT || last_screen == SCREEN_CARD_SUFFICIENT_AMOUNT || last_screen == SCREEN_CARD_SCAN || last_screen == SCREEN_BALANCE_INTRO || last_screen == SCREEN_PASS_INTRO || last_screen == SCREEN_PASS_FIRST )
            // 	  display_form( last_screen );
            // else
        	//    printf( "MORE TIME: %d\n", last_screen );

            break;

        case SCREEN_PIN_CODE:

        	if( card.type > 0  )
        	{
        		int trials = pin_code_match( card.pan, input_text );
        		if( trials == 0 )
        		{
        			if( card.type == 1 )
        			{
        				// unlock request
        				unlock_requested = UNLOCK_FINANCIAL;
        				if( transaction.session_amount > 0 )
        				{
        					curr_state = STATE_COLLECT;
							init_transaction( terminal.id, TRANSACTION_COLLECT, card.pan, card.uid, transaction.session_amount, 0 );
							activate_transaction();
							display_form( SCREEN_LOADING );
        				}
        				else
        					display_form( SCREEN_HOME );
        			}
        			else if( card.type == 2 )
        			{
        				// unlock request
        				unlock_requested = UNLOCK_MECHANICAL;
        				display_form( SCREEN_HOME );
        			}
        		}
        		else
        		{
        			if( trials > 2 )
        			{
        				save_service_cards();
        				display_form( SCREEN_HOME );
        			}
        			else
        			{
        				gtk_label_set_text( GTK_LABEL( value_labels[0] ), "" );
        				strcpy( input_text, "" );
        				gtk_widget_hide( next_button );

        				char buf[128];
        				sprintf( buf, "%s\n(%s: %d)", _("PinCode"), _("AttemptsLeft"), 3 - trials );
        				gtk_label_set_text( GTK_LABEL( hint_labels[0] ), buf );
        			}
        		}
        	}

            break;

        case SCREEN_MANUAL_INPUT:

            if( strlen( input_text ) == 19 )
            {
                reset_card_info();
                strcpy( card.pan, input_text );
                card.type = get_card_type( card.pan );
				if( card.type == 1 || card.type == 2 )
				{
					if( get_trials_count( card.pan ) < 3 )
					{
						card.initialized = TRUE;
						display_form( SCREEN_PIN_CODE );
					}
					else
						display_form( SCREEN_CARD_NOT_FOUND );
				}
				else
				{
					display_form( SCREEN_LOADING );
					card_activated();
				}
            }
            break;

        case SCREEN_PASS_INTRO:

        	display_form( SCREEN_MANUAL_INPUT );
            break;

        default:
        	trace( "next_callback default handler!" );
        	break;
    }
}

void lang_callback( GtkWidget* widget, gpointer data )
{
	guint lang_index = GPOINTER_TO_UINT( data );
	set_language( (enum APP_LANG)lang_index );

	//----------------- debug ------------------------->
	/*
	// if( lang_index == 2 )
	{
		// touch_instruction_callback( widget, NULL );
		print_cheque();
		return;
	}
	*/
	//----------------- debug -------------------------<
}

gboolean counter_timer_callback( gpointer data )
{
	if( counter_timer_id == 0 )
		return TRUE;

    time_counter ++;
    if( time_counter > MAX_COUNTER_VALUE )
    	time_counter = 0;

	switch( curr_screen )
	{
		case SCREEN_HOME:
			if( time_counter >= INTRO_PLAY_INTERVAL_SECONDS )
			{
				counter_timer_id = 0;

				if( operating() )
					display_form( SCREEN_INTRO );
				else
					display_form( SCREEN_OUT_OF_SERVICE );

				return FALSE;
			}
			break;

		case SCREEN_BALANCE_REFILLED:
		case SCREEN_BALANCE_REFILLED_WHEN_CONNECTED:
		case SCREEN_NO_CARDS:
		case SCREEN_PASS_ALREADY_ACTIVE:
		case SCREEN_PASS_SUFFICIENT_BALANCE:
		case SCREEN_PASS_INSUFFICIENT_AMOUNT:
		case SCREEN_PASS_ACTIVE:
		case SCREEN_PASS_ACTIVE_NEXT_MONTH:
		case SCREEN_PASS_ACTIVE_WHEN_CONNECTED:
		case SCREEN_CARD_INSUFFICIENT_AMOUNT:
		case SCREEN_CARD_FREE:
		case SCREEN_CARD_BOUGHT:
		case SCREEN_CARD_NOT_FOUND:

			if( time_counter >= AUTOCOMPLETE_SECONDS )
			{
				counter_timer_id = 0;
				display_form( SCREEN_HOME );
				return FALSE;
			}
			break;

		case SCREEN_OUT_OF_SERVICE:

			if( time_counter >= INTRO_PLAY_INTERVAL_SECONDS )
			{
				if( operating() && !pending_transaction() )
				{
					counter_timer_id = 0;
					display_form( SCREEN_HOME );
					return FALSE;
				}
			}
			break;

		case SCREEN_CARD_SCAN:
			if( time_counter >= INACTIVITY_SECONDS )
			{
				time_counter = 0; // this.ResetCounterTimer();
				display_more_time_form( COUNTDOWN_SECONDS );
			}
			else
				issue_sound_signal = 1;
			break;

		case SCREEN_LOADING:

			break;

		case SCREEN_CARD_INTRO:
		case SCREEN_CARD_PARTIAL_AMOUNT:
		case SCREEN_CARD_PRE_SCAN:
		case SCREEN_PASS_PARTIAL_AMOUNT:
		case SCREEN_BALANCE_INTRO:
		case SCREEN_PASS_INTRO:
		case SCREEN_PASS_FIRST:
		case SCREEN_PASS_SUFFICIENT_AMOUNT:
		case SCREEN_MANUAL_INPUT:

			if( time_counter >= INACTIVITY_SECONDS )
			{
				if( cash_retrieval_enabled )
					enable_cash_retrieval( FALSE );

				time_counter = 0; // this.ResetCounterTimer();
				display_more_time_form( COUNTDOWN_SECONDS );
			}
			break;

		case SCREEN_MORE_TIME:
			if( time_counter >= COUNTDOWN_SECONDS )
			{
				if( last_screen == SCREEN_CARD_PRE_SCAN )
					next_callback( next_button, NULL );
				else
					back_callback( back_button, NULL );
			}
			else
				display_more_time_form( COUNTDOWN_SECONDS - time_counter );
			break;

		default:
			break;
	}
	return TRUE;
}

void key_callback( GtkWidget *widget, gpointer data )
{
	int num = GPOINTER_TO_INT( data );
	char numc[2], buf[32];
	int len = strlen( input_text ), c = 0;

	if( num == -1 )
	{
		if( len > 0 )
		{
			input_text[len - 1] = 0;
			len -= 1;

			if( curr_screen == SCREEN_MANUAL_INPUT )
			{
				for( int i = 0; i < len; i++ )
				{
					if( i == 4 || i == 6 || i == 11 || i == 15 )
						buf[c++] = ' ';
					buf[c++] = input_text[i];
				}
				buf[c] = '\0';

				gtk_label_set_text( GTK_LABEL( value_labels[0] ), buf );
				if( len < 19 )
					gtk_widget_hide( next_button );
			}
			else if( curr_screen == SCREEN_PIN_CODE )
			{
				char pin[10];
				bzero( pin, 10 );
				for( int i = 0; i < len; i++ )
					strcat( pin, "*" );
				gtk_label_set_text( GTK_LABEL( value_labels[0] ), pin );

				if( len < 4 )
					gtk_widget_hide( next_button );
			}
		}

		return;
	}
	else if( ( len < 19 && curr_screen == SCREEN_MANUAL_INPUT ) || ( len < 4 && curr_screen == SCREEN_PIN_CODE ) )
	{
		sprintf( numc, "%u", num );
		strcat( input_text, numc );
		len += 1;

		if( curr_screen == SCREEN_MANUAL_INPUT )
		{
			for( int i = 0; i < len; i++ )
			{
				if( i == 4 || i == 6 || i == 11 || i == 15 )
					buf[c++] = ' ';
				buf[c++] = input_text[i];
			}
			buf[c] = '\0';

			gtk_label_set_text( GTK_LABEL( value_labels[0] ), buf );
			if( len == 19 )
				gtk_widget_show( next_button );
		}
		else if( curr_screen == SCREEN_PIN_CODE )
		{
			char pin[10];
			bzero( pin, 10 );
			for( int i = 0; i < len; i++ )
				strcat( pin, "*" );
			gtk_label_set_text( GTK_LABEL( value_labels[0] ), pin );

			if( len == 4 )
				gtk_widget_show( next_button );
		}
		// g_print( "key: %s\n", numc );
	}

	reset_counter_timer();
}

void wrapped_label_size_allocate_callback( GtkWidget* label, GtkAllocation* allocation, gpointer data )
{
     gtk_widget_set_size_request( label, allocation->width - 200, -1 );
}
// -------------- callbacks -------------------------------------------------------------<


// -------------- network --------------------------------------------------------------->

struct MemoryStruct
{
	char* memory;
	size_t size;
};

static size_t curl_write_memory_callback( void* contents, size_t size, size_t nmemb, void* userp )
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct*)userp;

	mem->memory = realloc( mem->memory, mem->size + realsize + 1 );
	if( mem->memory == NULL )
	{
		printf( "not enough memory (realloc returned NULL)\n" );
		return 0;
	}

	memcpy( &(mem->memory[mem->size] ), contents, realsize );
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

gboolean get_terminal_id()
{
	char data[256];
	strcpy( data, "id=" );
	strcat( data, terminal.serial );

	if( DEBUGGING )
		trace( data );

	CURL* curl = curl_easy_init();
	if( curl )
	{
		struct MemoryStruct chunk;
		chunk.memory = malloc( 1 ); // will be grown as needed by the realloc above
		chunk.size = 0;    			// no data at this point

		curl_easy_setopt( curl, CURLOPT_TIMEOUT, 30L ); // complete within 30 seconds
		curl_easy_setopt( curl, CURLOPT_URL, TERMINAL_ID_URL );
		curl_easy_setopt( curl, CURLOPT_POSTFIELDS, data );

		curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curl_write_memory_callback );
		curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void*)&chunk );

        curl_easy_setopt( curl, CURLOPT_DNS_USE_GLOBAL_CACHE, 0 );
        curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 30L );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );

		CURLcode res = curl_easy_perform( curl );
		if( res != CURLE_OK )
		{
			terminal.connected = FALSE;
			fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror( res ) );
		}
		else
		{
			if( DEBUGGING )
				printf( "\n\n%s\n\n", chunk.memory );

			terminal.id = atoi( chunk.memory );
		}

		curl_easy_cleanup( curl );
		free( chunk.memory );
	}
	return ( terminal.id > 0 );
}

gboolean get_terminal_info()
{
	gboolean result = FALSE;
	char data[256];

	sprintf( data, "id=%d&version=%d&os=%s", terminal.id, APP_VERSION, terminal.operating_system );

	if( DEBUGGING )
		trace( data );

	CURL* curl = curl_easy_init();
	if( curl )
	{
		struct MemoryStruct chunk;
		chunk.memory = malloc( 1 ); // will be grown as needed by the realloc above
		chunk.size = 0;    			// no data at this point

		curl_easy_setopt( curl, CURLOPT_TIMEOUT, 30L ); // complete within 30 seconds
		curl_easy_setopt( curl, CURLOPT_URL, TERMINAL_INFO_URL );
		curl_easy_setopt( curl, CURLOPT_POSTFIELDS, data );

		curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curl_write_memory_callback );
		curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void*)&chunk );

        curl_easy_setopt( curl, CURLOPT_DNS_USE_GLOBAL_CACHE, 0 );
        curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 30L );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );

		CURLcode res = curl_easy_perform( curl );
		if( res != CURLE_OK )
		{
			// net = FALSE;
			fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror( res ) );
		}
		else
		{
			if( DEBUGGING )
				printf( "\n\n%s\n\n", chunk.memory );

			const nx_json* json = nx_json_parse( chunk.memory, 0 );
			if( json )
			{
				terminal.update_to_version = nx_json_get( json, "update" )->int_value;
				terminal.update_interval = nx_json_get( json, "interval" )->int_value;
				terminal.card_price = nx_json_get( json, "card_price" )->int_value;
				terminal.command = nx_json_get( json, "cmd" )->int_value;
				const char* phone_number = nx_json_get( json, "phone_number" )->text_value;
				const char* phone_number_2 = nx_json_get( json, "phone_number_2" )->text_value;
				if( strlen( phone_number ) > 0 && strcmp( phone_number, terminal.phone_number ) != 0 )
				{
					strcpy( terminal.phone_number, phone_number );
					save_settings();
				}
				if( strlen( phone_number_2 ) > 0 )
					strcpy( terminal.phone_number_2, phone_number_2 );

			    // printf("KEY=%s\n", nx_json_get(nx_json_get(json, "obj"), "KEY")->text_value);

			    const nx_json* cards = nx_json_get( json, "cards" );
			    if( cards != NULL && cards->length > 0 )
			    {
			    	servicecards_count = cards->length;
			    	if( servicecards != NULL )
			    		servicecards = realloc( servicecards, servicecards_count * sizeof( struct service_card_info ) );
			    	else
			    		servicecards = malloc( servicecards_count * sizeof( struct service_card_info ) );

					for( int i = 0; i < servicecards_count; i++ )
					{
						const nx_json* item = nx_json_item( cards, i );
						if( item != NULL )
						{
							// trace( "Cards[%d]: %s %d %s", i, nx_json_get( item, "id" )->text_value, (int)nx_json_get( item, "type" )->int_value, nx_json_get( item, "pin" )->text_value );
							const nx_json* val = nx_json_get( item, "id" );
							if( val != NULL )
							{
								servicecards[i].trials = 0;
								strcpy( servicecards[i].pan, val->text_value );
								val = nx_json_get( item, "pin" );
								if( val != NULL )
								{
									strcpy( servicecards[i].pin, val->text_value );
									val = nx_json_get( item, "type" );
									if( val != NULL )
										servicecards[i].type = (int)val->int_value;
								}
							}
						}
					}
					save_service_cards();
			    }
				nx_json_free( json );
			}
			result = TRUE;
		}

		curl_easy_cleanup( curl );
		free( chunk.memory );
	}
	return result;
}

int64_t report_status()
{
	terminal.status = TERMINAL_OK;

	if( printer_status == PRINTER_OK )
	{
		if( paper_status == PAPER_LOW )
			terminal.status |= TERMINAL_LOW_PAPER;
		else if( paper_status == PAPER_EMPTY )
			terminal.status |= TERMINAL_NO_PAPER;
	}
	else
	{
		terminal.status |= TERMINAL_PRINTER_FAILED;
		trace( "PRINTER STATE: %d", printer_status );
	}

	if( dispenser_status == DISPENSER_DISPENSING_ERROR || dispenser_status == DISPENSER_EXECUTION_ERROR || dispenser_status == DISPENSER_NOT_READY || dispenser_status == DISPENSER_UNKNOWN_ERROR )
		terminal.status |= TERMINAL_DISPENSER_FAILED;

	if( stacker_status == STACK_LOW )
		terminal.status |= TERMINAL_LOW_CARDS;
	else if( stacker_status == STACK_EMPTY )
		terminal.status |= TERMINAL_NO_CARDS;

	if( ( bill_acceptor_status >= DROP_FULL && bill_acceptor_status <= CASSETTE_JAMMED ) || bill_acceptor_status == FAILURE || bill_acceptor_status == BILL_ACCEPTOR_UNKNOWN )
		terminal.status |= TERMINAL_BILL_ACCEPTOR_FAILED;

	if( card_reader_status > READER_OK )
		terminal.status |= TERMINAL_CARD_READER_FAILED;

	time_t now = time( NULL );
	if( transaction.amount > 0 && transaction.active && difftime( now, transaction.end ) > TRANSACTION_RESEND_INTERVAL_SECONDS )
		terminal.status |= TERMINAL_TRANSACTION_FAILED;

	int64_t time_updated = 0;
	char data[256];
	sprintf( data, "id=%d&amount=%d&status=%d&startup=%d", terminal.id, transaction.session_amount, terminal.status, terminal.startup );

	if( DEBUGGING )
		trace( data );

	CURL* curl = curl_easy_init();
	if( curl )
	{
		struct MemoryStruct chunk;
		chunk.memory = malloc( 1 ); // will be grown as needed by the realloc above
		chunk.size = 0;    			// no data at this point

		curl_easy_setopt( curl, CURLOPT_TIMEOUT, 30L ); // complete within 30 seconds
		curl_easy_setopt( curl, CURLOPT_URL, TERMINAL_STATUS_URL );
		curl_easy_setopt( curl, CURLOPT_POSTFIELDS, data );

		curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curl_write_memory_callback );
		curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void*)&chunk );

        curl_easy_setopt( curl, CURLOPT_DNS_USE_GLOBAL_CACHE, 0 );
        curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 30L );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );

		CURLcode res = curl_easy_perform( curl );
		if( res != CURLE_OK )
		{
			// net = FALSE;
			trace( "curl_easy_perform() failed: %s", curl_easy_strerror( res ) );
		}
		else
		{
			// printf( "\n\n%s\n\n", chunk.memory );
			time_updated = atoll( chunk.memory );
			// trace( "time_updated: %lld", time_updated );
			if( terminal.startup )
				terminal.startup = FALSE;
		}

		curl_easy_cleanup( curl );
		free( chunk.memory );
	}
	return time_updated;
}

gboolean commit_transaction()
{
	gboolean result = FALSE;
	char data[256], url[256], date[32];

    struct tm* tm_info = gmtime( &transaction.end );
    strftime( date, 32, "%d%m%y%H%M%S", tm_info );

	switch( transaction.type )
	{
	case TRANSACTION_REFILL:
		strcpy( url, REFILL_BALANCE_URL );
		sprintf( data, "cardId=%s&transId=%s&amount=%d&date=%s&status=%d", transaction.pan, transaction.id, transaction.amount, date, transaction.status );
		break;

	case TRANSACTION_BUY_CARD:
		strcpy( url, BUY_CARD_URL );
		sprintf( data, "cardId=%s&transId=%s&amount=%d&cost=%d&date=%s&status=%d", transaction.pan, transaction.id, transaction.amount, transaction.cost, date, transaction.status );
		break;

	case TRANSACTION_BUY_PASS:
		strcpy( url, BUY_PASS_URL );
		sprintf( data, "cardId=%s&transId=%s&amount=%d&cost=%d&date=%s&status=%d", transaction.pan, transaction.id, transaction.amount, transaction.cost, date, transaction.status );
		break;

	case TRANSACTION_COLLECT:
		strcpy( url, COLLECT_URL );
		sprintf( data, "cardId=%s&transId=%s&amount=%d&date=%s&status=%d", transaction.pan, transaction.id, transaction.amount, date, transaction.status );
		break;
	};

	if( DEBUGGING )
		trace( data );

	CURL* curl = curl_easy_init();
	if( curl )
	{
		struct MemoryStruct chunk;
		chunk.memory = malloc( 1 ); // will be grown as needed by the realloc above
		chunk.size = 0;    			// no data at this point

		curl_easy_setopt( curl, CURLOPT_TIMEOUT, 30L ); // complete within 30 seconds
		curl_easy_setopt( curl, CURLOPT_URL, url );
		curl_easy_setopt( curl, CURLOPT_POSTFIELDS, data );

		curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curl_write_memory_callback );
		curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void*)&chunk );

        curl_easy_setopt( curl, CURLOPT_DNS_USE_GLOBAL_CACHE, 0 );
        curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 30L );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );

		CURLcode res = curl_easy_perform( curl );
		transaction.status = TRANSACTION_FAILED;

		if( res != CURLE_OK )
		{
			save_transaction();
			fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror( res ) );
		}
		else
		{
			if( DEBUGGING )
				trace( "\n\n%s\n", chunk.memory );

			const nx_json* json = nx_json_parse( chunk.memory, 0 );
			if( json->type != NX_JSON_NULL )
			{
				const nx_json* val = nx_json_get( json, "status" );
				if( val->type != NX_JSON_NULL  )
				{
					transaction.status = val->int_value;
					if( transaction.status > -1 )
					{
						const char* fz_number = nx_json_get( json, "fiscal" )->text_value;
						strcpy( transaction.fz_number, fz_number );
						result = TRUE;
					}
				}
			}
			save_transaction();
			nx_json_free( json );
		}

		curl_easy_cleanup( curl );
		free( chunk.memory );
	}
	return result;
}

gboolean get_card_info()
{
	char url[256];
	strcpy( url, CARD_INFO_URL );
	strcat( url, card.pan );
	int res_code = -1;

	CURL* curl = curl_easy_init();
	if( curl )
	{
		struct MemoryStruct chunk;
		chunk.memory = malloc( 1 ); // will be grown as needed by the realloc above
		chunk.size = 0;    			// no data at this point

		curl_easy_setopt( curl, CURLOPT_TIMEOUT, 30L ); // complete within 30 seconds
		curl_easy_setopt( curl, CURLOPT_URL, url );

		curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curl_write_memory_callback );
		curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void*)&chunk );

        curl_easy_setopt( curl, CURLOPT_DNS_USE_GLOBAL_CACHE, 0 );
        curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 30L );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );

		CURLcode res = curl_easy_perform( curl );
		if( res != CURLE_OK )
		{
			fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror( res ) );
			card.status = CARD_NETWORK_ERROR;
		}
		else
		{
			if( DEBUGGING )
				trace( "\n\n%s\n", chunk.memory );

			const nx_json* json = nx_json_parse( chunk.memory, 0 );
			if( json )
			{
				// res_code = parseJsonInt( chunk.memory, "code" );
				res_code = nx_json_get( json, "code" )->int_value;
				if( res_code == 200 )
				{
					/*
					card.balance = parseJsonInt( chunk.memory, "balance" );
					card.pass_price = parseJsonInt( chunk.memory, "pass_price" );
					card.ride_price = parseJsonInt( chunk.memory, "ride_price" );

					char* pass_beg = parseJsonStr( chunk.memory, "pass_beg" );
					char* pass_end = parseJsonStr( chunk.memory, "pass_end" );
					*/

					card.balance = nx_json_get( json, "balance" )->int_value;
					card.pass_price = nx_json_get( json, "pass_price" )->int_value;
					card.ride_price = nx_json_get( json, "ride_price" )->int_value;

					const char* pass_beg = nx_json_get( json, "pass_beg" )->text_value;
					const char* pass_end = nx_json_get( json, "pass_end" )->text_value;

					if( strlen( pass_beg ) > 0 && strlen( pass_end ) > 0 )
					{
						struct tm tm_info = { 0 };
						int year, month, day;
						if( sscanf( pass_beg, "%d-%d-%d", &day, &month, &year ) != EOF )
						{
							tm_info.tm_year = year - 1900;
							tm_info.tm_mon  = month - 1;
							tm_info.tm_mday = day;
							card.pass_beg = mktime( &tm_info );

							if( sscanf( pass_end, "%d-%d-%d", &day, &month, &year ) != EOF )
							{
								tm_info.tm_year = year - 1900;
								tm_info.tm_mon  = month - 1;
								tm_info.tm_mday = day;
								card.pass_end = mktime( &tm_info );
								// trace( "%s - %s\n", pass_beg, pass_end );
							}
						}
					}
					else
					{
						card.pass_beg = 0;
						card.pass_end = 0;
					}

					card.status = CARD_OK;
				}
				else if( res_code == 404 )
				{
					card.status = CARD_NOT_FOUND;
				}
				nx_json_free( json );
			}
			// trace( "\n\n%s\n", chunk.memory );
		}

		curl_easy_cleanup( curl );
		free( chunk.memory );
	}
	return ( card.status == CARD_OK );
}
// -------------- network ---------------------------------------------------------------<



// -------------- ui -------------------------------------------------------------------->

GtkWidget* create_main_window()
{
	GtkWidget* window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	g_signal_connect( window, "destroy", G_CALLBACK( quit ), NULL );

	gtk_widget_set_size_request( window, APP_WIDTH, APP_HEIGHT );
	gtk_window_set_position( GTK_WINDOW( window ), GTK_WIN_POS_CENTER );
	gtk_window_set_decorated( GTK_WINDOW( window ), FALSE );
	gtk_window_set_resizable( GTK_WINDOW( window ), FALSE );
	gtk_widget_modify_bg( window, GTK_STATE_NORMAL, &onay_color );

	GtkSettings* default_settings = gtk_settings_get_default();
	g_object_set( default_settings, "gtk-button-images", TRUE, NULL );

	return window;
}

void create_ui()
{
	main_window = create_main_window();
}

void remove_last_form()
{
	if( curr_form != NULL )
	{
		gtk_container_remove( GTK_CONTAINER( main_window ), curr_form );
		curr_form = NULL;
	}
}

GtkWidget* create_lang_panel()
{
	GtkWidget* hbox = gtk_hbox_new( TRUE, 0 );

	GtkWidget* label;
	GtkWidget* button;
	const char* const captions[] = { "Қазақша", "Русский", "English" };

	for( int i = 0; i < 3; i++ )
	{
		button = gtk_button_new();
		if( lang == i )
		{
			gtk_widget_modify_bg( button, GTK_STATE_NORMAL, &onay_highl_color );
			gtk_widget_modify_bg( button, GTK_STATE_ACTIVE, &onay_highl_color );
			gtk_widget_modify_bg( button, GTK_STATE_PRELIGHT, &onay_highl_color );
		}
		else
		{
			gtk_widget_modify_bg( button, GTK_STATE_NORMAL, &onay_color );
			gtk_widget_modify_bg( button, GTK_STATE_ACTIVE, &onay_color );
			gtk_widget_modify_bg( button, GTK_STATE_PRELIGHT, &onay_color );
		}
		gtk_widget_set_can_focus( button, FALSE );
		gtk_widget_set_size_request( button, BTN_WIDTH, BTN_HEIGHT );
		g_signal_connect( button, "clicked", G_CALLBACK( lang_callback ), GINT_TO_POINTER( i ) );

		label = gtk_label_new( captions[i] );
		gtk_widget_modify_fg( label, GTK_STATE_NORMAL, &black_color );
		gtk_widget_modify_fg( label, GTK_STATE_PRELIGHT, &black_color );
		gtk_widget_modify_font( label, font_buttons );
		gtk_container_add( GTK_CONTAINER( button ), label );
		gtk_widget_show( label );

		gtk_box_pack_start( GTK_BOX( hbox ), button, TRUE, TRUE, 0 );
		// gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
		gtk_widget_show( button );
	}
	return hbox;
}

GtkWidget* create_generic_button( const char* text )
{
	GtkWidget* button = gtk_button_new();
	gtk_widget_modify_bg( button, GTK_STATE_NORMAL, &onay_color );
	gtk_widget_modify_bg( button, GTK_STATE_ACTIVE, &onay_color );
	gtk_widget_modify_bg( button, GTK_STATE_PRELIGHT, &onay_color );
	gtk_widget_set_can_focus( button, FALSE );
	gtk_widget_set_size_request( button, BTN_WIDTH, BTN_HEIGHT );

	GtkWidget* label = gtk_label_new( text );
	gtk_widget_modify_fg( label, GTK_STATE_NORMAL, &black_color );
	gtk_widget_modify_fg( label, GTK_STATE_PRELIGHT, &black_color );
	gtk_widget_modify_font( label, font_buttons );
	gtk_label_set_line_wrap( GTK_LABEL( label ), TRUE );
	gtk_label_set_justify( GTK_LABEL( label ), GTK_JUSTIFY_CENTER );
	gtk_label_set_width_chars( GTK_LABEL( label ), 18 );
	gtk_container_add( GTK_CONTAINER( button ), label );
	gtk_widget_show( label );

	return button;
}

GtkWidget* create_generic_label( const char* text, PangoFontDescription* font )
{
	GtkWidget* label = gtk_label_new( text );
	gtk_widget_modify_fg( label, GTK_STATE_NORMAL, &black_color );
	gtk_widget_modify_fg( label, GTK_STATE_PRELIGHT, &black_color );

	gtk_widget_modify_font( label, font );
	gtk_label_set_line_wrap( GTK_LABEL( label ), TRUE );
	gtk_label_set_justify( GTK_LABEL( label ), GTK_JUSTIFY_CENTER );
	return label;
}

GtkWidget* create_generic_form( guint label_rows, const char* instruction_text, const char* back_text, const char* mid_text, const char* next_text, gboolean arrow )
{
	GtkWidget* form = gtk_vbox_new( FALSE, 10 );

	GtkWidget* hbox = create_lang_panel();
	gtk_box_pack_start( GTK_BOX( form ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	for( guint i = 0; i < label_rows && i < 3; i++ )
	{
		// hbox = gtk_hbox_new( FALSE, 0 );
		hbox = gtk_fixed_new();

		hint_labels[i] = gtk_label_new( "" );
		gtk_widget_modify_fg( hint_labels[i], GTK_STATE_NORMAL, &black_color );
		gtk_widget_modify_fg( hint_labels[i], GTK_STATE_PRELIGHT, &black_color );

		gtk_widget_modify_font( hint_labels[i], font_labels );
		// gtk_misc_set_alignment( GTK_MISC( hint_labels[i] ), 0.0, 0.5 );

		// gtk_box_pack_start( GTK_BOX( hbox ), hint_labels[i], FALSE, FALSE, HINT_PADDING );
		gtk_fixed_put( GTK_FIXED( hbox ), hint_labels[i], HINT_OFFSET, i );
		gtk_widget_show( hint_labels[i] );

		value_labels[i] = gtk_label_new( "" );
		gtk_widget_modify_fg( value_labels[i], GTK_STATE_NORMAL, &black_color );
		gtk_widget_modify_fg( value_labels[i], GTK_STATE_PRELIGHT, &black_color );
		gtk_widget_modify_font( value_labels[i], font_labels );
		// gtk_box_pack_start( GTK_BOX( hbox ), value_labels[i], FALSE, FALSE, 0 );
		gtk_fixed_put( GTK_FIXED( hbox ), value_labels[i], LABEL_OFFSET, i );
		gtk_widget_show( value_labels[i] );

		gtk_box_pack_start( GTK_BOX( form ), hbox, FALSE, FALSE, 5 );
		gtk_widget_show( hbox );
	}

	hbox = gtk_hbox_new( FALSE, 0 );
	instructions_label = create_generic_label( instruction_text, font_instructions );
	g_signal_connect( G_OBJECT( instructions_label ), "size-allocate", G_CALLBACK( wrapped_label_size_allocate_callback ), NULL );
	gtk_label_set_width_chars( GTK_LABEL( instructions_label ), 64 );
	// gtk_misc_set_alignment( GTK_MISC( instructions_label ), 0.5, 0.5 );
	gtk_box_pack_start( GTK_BOX( form ), instructions_label, TRUE, FALSE, 0 );
	gtk_widget_show( instructions_label );

	if( back_text != NULL || next_text != NULL || mid_text != NULL || arrow )
	{
		hbox = gtk_hbox_new( FALSE, 0 );

		if( back_text != NULL )
		{
			back_button = create_generic_button( back_text );
			g_signal_connect( back_button, "clicked", G_CALLBACK( back_callback ), NULL );
			if( mid_text != NULL )
				gtk_box_pack_start( GTK_BOX( hbox ), back_button, TRUE, TRUE, 0 );
			else
				gtk_box_pack_start( GTK_BOX( hbox ), back_button, FALSE, FALSE, 0 );
			gtk_widget_show( back_button );
		}

		if( mid_text != NULL )
		{
			GtkWidget* mid_button = create_generic_button( mid_text );
			g_signal_connect( mid_button, "clicked", G_CALLBACK( mid_callback ), NULL );
			gtk_box_pack_start( GTK_BOX( hbox ), mid_button, TRUE, TRUE, 0 );
			gtk_widget_show( mid_button );
		}
		else if( arrow )
		{
			GdkPixbufAnimation* animation = load_animation_from_resource( "/arrow.gif" ); // gdk_pixbuf_animation_new_from_file( "arrow.gif", NULL );
			GtkWidget* arrow_image = gtk_image_new();
			gtk_image_set_from_animation( GTK_IMAGE( arrow_image ), animation );
			gtk_box_pack_start( GTK_BOX( hbox ), arrow_image, TRUE, FALSE, 5 );
			gtk_widget_show( arrow_image );
		}

		if( next_text != NULL )
		{
			next_button = create_generic_button( next_text );
			g_signal_connect( next_button, "clicked", G_CALLBACK( next_callback ), NULL );
			if( mid_text != NULL )
				gtk_box_pack_end( GTK_BOX( hbox ), next_button, TRUE, TRUE, 0 );
			else
				gtk_box_pack_end( GTK_BOX( hbox ), next_button, FALSE, FALSE, 0 );
			gtk_widget_show( next_button );
		}

		gtk_box_pack_end( GTK_BOX( form ), hbox, FALSE, FALSE, 0 );
		gtk_widget_show( hbox );
	}
	else
	{
		GtkWidget* label = gtk_label_new( "" );
		gtk_widget_set_size_request( label, BTN_WIDTH, BTN_HEIGHT );
		gtk_box_pack_start( GTK_BOX( form ), label, FALSE, FALSE, 0 );
		gtk_widget_show( label );
	}

	return form;
}

GtkWidget* create_intro_form()
{
	GtkWidget* form = gtk_drawing_area_new ();
	g_signal_connect( form, "realize", G_CALLBACK( video_widget_realize_cb ), NULL );
	g_signal_connect( form, "button_press_event", G_CALLBACK( touch_intro_callback ), NULL );
	gtk_widget_set_double_buffered( form, FALSE );
	gtk_widget_set_events( form, GDK_BUTTON_PRESS_MASK );

	if( lang == LANG_KZ )
		lang = LANG_RU;
	else
		lang = LANG_KZ;
	init_locale();

	return form;
}

GtkWidget* create_home_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 0, _("HomeInstruction"), _("GetCard"), _("ManualInput"), _("GetPass"), FALSE );

	/*
	GtkWidget* form = gtk_vbox_new( FALSE, 10 );
	GtkWidget* hbox = create_lang_panel();

	gtk_box_pack_start( GTK_BOX( form ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	GtkWidget* logo_image = gtk_image_new_from_pixbuf( load_image_from_resource( "/onay_logo.png" ) );
	gtk_box_pack_start( GTK_BOX( form ), logo_image, FALSE, FALSE, 10 );
	gtk_widget_show( logo_image );

	GtkWidget* label = create_generic_label( _("HomeInstruction"), font_instructions );
	g_signal_connect( G_OBJECT( label ), "size-allocate", G_CALLBACK( wrapped_label_size_allocate_callback ), NULL );
	gtk_label_set_width_chars( GTK_LABEL( label ), 64 );
	gtk_box_pack_start( GTK_BOX( form ), label, TRUE, FALSE, 0 );
	gtk_widget_show( label );

	const char* const captions2[] = { _("GetCard"), _("ManualInput"), _("GetPass") };
	hbox = gtk_hbox_new( FALSE, 0 );
	GtkWidget* button = NULL;
	for( int i = 0; i < 3; i++ )
	{
		button = create_generic_button( captions2[i] );
		if( i == 0 )
			g_signal_connect( button, "clicked", G_CALLBACK( back_callback ), NULL );
		else if( i == 1 )
			g_signal_connect( button, "clicked", G_CALLBACK( mid_callback ), NULL );
		else
			g_signal_connect( button, "clicked", G_CALLBACK( next_callback ), NULL );
		gtk_box_pack_start( GTK_BOX( hbox ), button, TRUE, TRUE, 0 );
		gtk_widget_show( button );
	}
	gtk_box_pack_end( GTK_BOX( form ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );
	*/

	if( !reload_ui )
		clear_state();
	reset_counter_timer();

	return form;
}

GtkWidget* create_no_cards_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 0, _("NoCardsInstruction"), _("Exit"), NULL, NULL, FALSE );
	if( !reload_ui )
		reset_counter_timer();

	return form;
}

GtkWidget* create_pass_intro_form( gboolean reload_ui )
{
    char instruction[1024];
    char month[32];
    char month2[32];

    time_t timer = time( NULL );
    struct tm* tm_info = localtime( &timer );

    int today = tm_info->tm_mday;
    strcpy( month, MONTH_NAMES[ tm_info->tm_mon + (12 * lang)] );

    if( tm_info->tm_mon == 11 )
    {
    	tm_info->tm_mon = 0;
    	tm_info->tm_year += 1;
    }
    else
    	tm_info->tm_mon += 1;

    strcpy( month2, MONTH_NAMES[ tm_info->tm_mon + (12 * lang)] );

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"
    if( today >= 15 )
    	sprintf( instruction, _("PassIntroInstruction2"), month2 );
    else
    	sprintf( instruction, _("PassIntroInstruction"), month, month2 );
#pragma GCC diagnostic pop

	GtkWidget* form = create_generic_form( 0, instruction, _("Cancel"), NULL, _("ManualInput"), TRUE );
	if( !reload_ui )
		reset_counter_timer();

	return form;
}

void init_pass_labels()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"
	gtk_label_set_text( GTK_LABEL( hint_labels[0] ), _("PassPrice") );
	gtk_label_set_text( GTK_LABEL( hint_labels[1] ), _("MinimumBalance") );
	gtk_label_set_text( GTK_LABEL( hint_labels[2] ), _("CurrentBalance") );

	char val[64];
	sprintf( val, _("AmountValue"), (int)card.pass_price );
	gtk_label_set_text( GTK_LABEL( value_labels[0] ), val );

	sprintf( val, _("AmountValue"), (int)card.ride_price );
	gtk_label_set_text( GTK_LABEL( value_labels[1] ), val );

	sprintf( val, _("AmountValue"), (int)( card.balance + transaction.amount ) );
	gtk_label_set_text( GTK_LABEL( value_labels[2] ), val );

	int required_amount = (int)( card.pass_price + card.ride_price - ( transaction.amount + card.balance ) );
	char instruction[1024];
	if( required_amount > 0 )
	{
		sprintf( instruction, _("DepositMinimumAmount"), required_amount );
		strcat( instruction, " " );
		strcat( instruction, _("PassFirstInstruction") );
	}
	else
		strcpy( instruction, _("PassFirstInstruction") );
	gtk_label_set_text( GTK_LABEL( instructions_label ), instruction );
#pragma GCC diagnostic pop

	/*
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"
	gtk_label_set_text( GTK_LABEL( hint_labels[0] ), _("PassPrice") );
	gtk_label_set_text( GTK_LABEL( hint_labels[1] ), _("AmountDeposited") );

	char val[64];
	sprintf( val, _("PassPriceValue"), (int)card.pass_price, (int)card.ride_price );

	// trace( val );
	// return;

	gtk_label_set_text( GTK_LABEL( value_labels[0] ), val );

	int refill_amount = (int)( transaction.amount + card.balance - card.pass_price );

	if( ( transaction.amount + card.balance ) >= ( card.pass_price + card.ride_price ) )
	{
		gtk_label_set_text( GTK_LABEL( hint_labels[2] ), _("Balance") );

		sprintf( val, _("AmountValue"), (int)card.pass_price );
		gtk_label_set_text( GTK_LABEL( value_labels[1] ), val );
		sprintf( val, _("AmountValue"), refill_amount );
		gtk_label_set_text( GTK_LABEL( value_labels[2] ), val );
	}
	else
	{
		gtk_label_set_text( GTK_LABEL( hint_labels[2] ), _("BalanceToRefill") );

		sprintf( val, _("AmountValue"), (int)( transaction.amount + card.balance ) );
		gtk_label_set_text( GTK_LABEL( value_labels[1] ), val );

		if( refill_amount > 0 )
			sprintf( val, _("AmountValue"), refill_amount );
		else
			sprintf( val, _("AmountValue"), 0 );
	}
#pragma GCC diagnostic pop
    */
}

GtkWidget* create_pass_first_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 3, _("PassFirstInstruction"), _("Cancel"), NULL, NULL, FALSE );
	init_pass_labels();
	if( !reload_ui )
	{
		if( !cash_retrieval_enabled )
			enable_cash_retrieval( TRUE );
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_pass_already_active_form( gboolean reload_ui )
{
    char instruction[256];
    char month[16];
    char month2[16];

    gboolean next_month_notice = FALSE;
    time_t timer = time( NULL );
    struct tm* tm_info = localtime( &timer );
    int today = tm_info->tm_mday;
    strcpy( month, MONTH_NAMES[ tm_info->tm_mon + (12 * lang)] );

    if( tm_info->tm_mon == 11 )
    {
    	tm_info->tm_mon = 0;
    	tm_info->tm_year += 1;
    }
    else
    	tm_info->tm_mon += 1;
	tm_info->tm_mday = 1;

    strcpy( month2, MONTH_NAMES[ tm_info->tm_mon + (12 * lang)] );

    if( today >= 15 )
        strcpy( month, month2 );
    else
    {
        if( !card_contains_valid_pass( mktime( tm_info ) ) )
        	next_month_notice = TRUE;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"
    if( next_month_notice )
    	sprintf( instruction, _("PassAlreadyActiveInstruction2"), month, month2 );
    else
    	sprintf( instruction, _("PassAlreadyActiveInstruction"), month );
#pragma GCC diagnostic pop

	GtkWidget* form = create_generic_form( 0, instruction, _("Exit"), NULL, NULL, FALSE );

	if( !reload_ui )
	{
		reset_counter_timer();
	}

	return form;
}

GtkWidget* create_pass_partial_amount_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 3, _("PassFirstInstruction"), NULL, NULL, NULL, FALSE );
	init_pass_labels();
	if( !reload_ui )
	{
		if( !cash_retrieval_enabled )
			enable_cash_retrieval( TRUE );
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_pass_sufficient_balance_form( gboolean reload_ui )
{
	GtkWidget* form = NULL;
	if( pending_transaction() )
		form = create_generic_form( 3, _("PassFirstInstruction"), NULL, NULL, _("Next"), FALSE );
	else
		form = create_generic_form( 3, _("PassFirstInstruction"), _("Cancel"), NULL, _("Next"), FALSE );
	init_pass_labels();
	if( !reload_ui )
	{
		if( !cash_retrieval_enabled )
			enable_cash_retrieval( TRUE );
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_pass_sufficient_amount_form( gboolean reload_ui )
{
	GtkWidget* form = NULL;
	if( pending_transaction() )
		form = create_generic_form( 3, _("PassFirstInstruction"), NULL, NULL, _("Next"), FALSE );
	else
		form = create_generic_form( 3, _("PassFirstInstruction"), _("Cancel"), NULL, _("Next"), FALSE );
	init_pass_labels();
	if( !reload_ui )
	{
		if( !cash_retrieval_enabled )
			enable_cash_retrieval( TRUE );
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_pass_insufficient_amount_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 0, _("PassInsufficientAmountInstruction"), _("Exit"), NULL, NULL, FALSE );
	if( !reload_ui )
	{
		if( transaction.amount > 0 )
			activate_transaction();
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_pass_active_form( gboolean reload_ui )
{
    time_t now = time( NULL );
    struct tm* tm_info = localtime( &now );

    char instruction[1024];
    char month[32];

    if( tm_info->tm_mday > 14 )
    {
        if( tm_info->tm_mon == 11 )
        {
        	tm_info->tm_mon = 0;
        	tm_info->tm_year += 1;
        }
        else
        	tm_info->tm_mon += 1;
    }
    strcpy( month, MONTH_NAMES[ tm_info->tm_mon + (12 * lang)] );

    int new_balance = (int)( card.balance + print_transaction.amount - card.pass_price );

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"
    sprintf( instruction, _("PassActiveInstruction"), month, new_balance );
#pragma GCC diagnostic pop

	GtkWidget* form = create_generic_form( 0, instruction, _("Exit"), NULL, NULL, FALSE );
	if( !reload_ui )
	{
		print_cheque();
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_pass_active_next_month_form( gboolean reload_ui )
{
    char instruction[256];
    char month[16];
    time_t timer = time( NULL );
    struct tm* tm_info = localtime( &timer );

    if( tm_info->tm_mon == 11 )
    {
    	tm_info->tm_mon = 0;
    	tm_info->tm_year += 1;
    }
    else
    	tm_info->tm_mon += 1;
    strcpy( month, MONTH_NAMES[ tm_info->tm_mon + (12 * lang)] );
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"
    sprintf( instruction, _("PassActiveNextMonthInstruction"), month );
#pragma GCC diagnostic pop

	GtkWidget* form = create_generic_form( 0, instruction, _("Exit"), NULL, NULL, FALSE );
	if( !reload_ui )
	{
		print_cheque();
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_pass_active_when_connected_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 0, _("PassActiveWhenConnectedInstruction"), _("Exit"), NULL, NULL, FALSE );
	if( !reload_ui )
	{
		print_cheque();
		reset_counter_timer();
	}
	return form;
}

void init_card_labels( gboolean scan )
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"

	gtk_label_set_text( GTK_LABEL( hint_labels[0] ), _("CardPrice") );
	gtk_label_set_text( GTK_LABEL( hint_labels[1] ), _("AmountDeposited") );
	gtk_label_set_text( GTK_LABEL( hint_labels[2] ), _("BalanceToRefill") );

	char val[32];
	sprintf( val, _("AmountValue"), (int)terminal.card_price );
	gtk_label_set_text( GTK_LABEL( value_labels[0] ), val );
	sprintf( val, _("AmountValue"), (int)transaction.amount );
	gtk_label_set_text( GTK_LABEL( value_labels[1] ), val );
	if( transaction.amount <= terminal.card_price )
		sprintf( val, _("AmountValue"), 0 );
	else
		sprintf( val, _("AmountValue"), (int)( transaction.amount - terminal.card_price ) );
	gtk_label_set_text( GTK_LABEL( value_labels[2] ), val );

	int required_amount = (int)( terminal.card_price - transaction.amount );
	char instruction[1024];
	if( required_amount > 0 )
	{
		sprintf( instruction, _("DepositMinimumAmount"), required_amount );
		strcat( instruction, " " );
		strcat( instruction, _("CardIntroInstruction") );
	}
	else
	{
		if( scan )
			strcpy( instruction, _("CardScanInstruction") );
		else
		{
			if( required_amount == 0 )
				strcpy( instruction, _("CardIntroInstruction") );
			else
				sprintf( instruction, _("CardPreScanInstruction"), abs( required_amount ) );
		}
	}
	gtk_label_set_text( GTK_LABEL( instructions_label ), instruction );
#pragma GCC diagnostic pop
}

GtkWidget* create_card_intro_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 3, _("CardIntroInstruction"), _("Cancel"), NULL, NULL, FALSE );
	init_card_labels( FALSE );

	if( !reload_ui )
	{
		if( !cash_retrieval_enabled )
			enable_cash_retrieval( TRUE );
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_card_partial_amount_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 3, _("CardIntroInstruction"), NULL, NULL, NULL, FALSE );
	init_card_labels( FALSE );

	if( !reload_ui )
	{
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_card_sufficient_amount_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 3, _("CardIntroInstruction"), NULL, NULL, _("Next"), FALSE );
	init_card_labels( FALSE );

	if( !reload_ui )
	{
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_card_insufficient_amount_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 0, _("CardInsufficientAmountInstruction"), _("Exit"), NULL, NULL, FALSE );

	if( !reload_ui )
	{
		print_cheque();
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_card_free_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 0, _("CardFreeInstruction"), _("Exit"), NULL, NULL, FALSE );
	if( !reload_ui )
	{
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_card_pre_scan_form( gboolean reload_ui )
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"

	char instruction[1024];
	int refill_amount = (int)( transaction.amount - terminal.card_price );
	if( refill_amount > 0 )
		sprintf( instruction, _("CardPreScanInstruction"), refill_amount );

	GtkWidget* form = create_generic_form( 0, instruction, NULL, NULL, _("Next"), FALSE );

	if( !reload_ui )
	{
		reset_counter_timer();
	}
	return form;
#pragma GCC diagnostic pop
}

GtkWidget* create_card_scan_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 3, _("CardScanInstruction"), NULL, NULL, NULL, TRUE );
	init_card_labels( TRUE );

	if( !reload_ui )
	{
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_loading_form()
{
	GtkWidget* form = gtk_vbox_new( FALSE, 10 );

	GtkWidget* spinner = gtk_spinner_new();
	gtk_widget_set_size_request( spinner, 80, 80 );
	gtk_widget_modify_fg( spinner, GTK_STATE_NORMAL, &black_color );
	gtk_box_pack_start( GTK_BOX( form ), spinner, TRUE, FALSE, 0 );
	gtk_widget_show( spinner );
	gtk_spinner_start( GTK_SPINNER( spinner ) );

	return form;
}

void init_balance_labels()
{
	gtk_label_set_text( GTK_LABEL( hint_labels[0] ), _("Balance") );
	gtk_label_set_text( GTK_LABEL( hint_labels[1] ), _("AmountDeposited") );

	char val[16];
	sprintf( val, "%d", (int)card.balance );
	gtk_label_set_text( GTK_LABEL( value_labels[0] ), val );
	sprintf( val, "%d", transaction.amount );
	gtk_label_set_text( GTK_LABEL( value_labels[1] ), val );
}

GtkWidget* create_balance_intro_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 2, _("BalanceIntroInstruction"), _("Exit"), NULL, NULL, FALSE );
	init_balance_labels();

	if( !reload_ui )
	{
		if( !cash_retrieval_enabled )
			enable_cash_retrieval( TRUE );
		reset_counter_timer();
	}

	return form;
}

GtkWidget* create_balance_refill_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 2, _("BalanceRefillInstruction"), NULL, NULL, _("Next"), FALSE );
	init_balance_labels();

	if( !reload_ui )
	{
		reset_counter_timer();
	}

	return form;
}

GtkWidget* create_balance_refilled_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 0, _("CardRefilledInstruction"), _("Exit"), NULL, NULL, FALSE );
	if( !reload_ui )
	{
		print_cheque();
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_balance_refilled_when_connected_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 0, _("CardRefilledWhenConnectedInstruction"), _("Exit"), NULL, NULL, FALSE );
	if( !reload_ui )
	{
		print_cheque();
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_card_not_found_form( gboolean reload_ui )
{
	GtkWidget* form = create_generic_form( 0, _("CardNotFoundInstruction"), _("Exit"), NULL, NULL, FALSE );
	if( !reload_ui )
	{
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_card_bought_form( gboolean reload_ui )
{
	char instruction[256];
	strcpy( instruction, _("ThankYouForPurchase") );
	if( transaction.amount > transaction.cost )
	{
		strcat( instruction, _("Space") );
		strcat( instruction, _("CardRefilledInstruction") );
	}

	GtkWidget* form = create_generic_form( 0, instruction, _("Exit"), NULL, NULL, FALSE );
	if( !reload_ui )
	{
		print_cheque();
		reset_counter_timer();
	}
	return form;
}

GtkWidget* create_out_of_service_form( gboolean reload_ui )
{
	/*
	GtkWidget* form = NULL;
	if( terminal.phone_number != NULL && strlen( terminal.phone_number ) > 0 )
	{
		// trace( "Phone number: %s", terminal.phone_number );
		form = create_generic_form( 2, _("OutOfServiceInstruction"), NULL, NULL, NULL, FALSE );

		gtk_label_set_text( GTK_LABEL( hint_labels[1] ), _("ContactPhoneNumber") );
		gtk_label_set_text( GTK_LABEL( value_labels[1] ), terminal.phone_number );

		GtkWidget* label = create_generic_label( "", font_instructions );
		gtk_box_pack_start( GTK_BOX( form ), label, FALSE, FALSE, 40 );
		gtk_widget_show( label );
	}
	else
	{
		// trace( "OutOfService without phone number" );
		form = create_generic_form( 0, _("OutOfServiceInstruction"), NULL, NULL, NULL, FALSE );
	}
	*/

	char instruction[1024];
	strcpy( instruction, _("OutOfServiceInstruction") );
	if( terminal.phone_number != NULL && strlen( terminal.phone_number ) > 0 )
	{
		strcat( instruction, ". " );
		strcat( instruction, _("SupportPhoneNumber") );
		strcat( instruction, terminal.phone_number );
	}
	GtkWidget* form = create_generic_form( 0, instruction, NULL, NULL, NULL, FALSE );

	if( !reload_ui )
	{
		curr_state = STATE_OUT_OF_SERVICE;
		clear_state();
		stop_counter_timer();
	}
	return form;
}

GtkWidget* create_more_time_form( int seconds_remain )
{
	GtkWidget* form = gtk_vbox_new( FALSE, 10 );
	GtkWidget* hbox = create_lang_panel();

	gtk_box_pack_start( GTK_BOX( form ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	GtkWidget* label = create_generic_label( _("MoreTimeNeeded"), font_instructions );
	gtk_box_pack_start( GTK_BOX( form ), label, FALSE, FALSE, 40 );
	gtk_widget_show( label );

	char seconds[8];
	sprintf( seconds, "%d", seconds_remain );
	instructions_label = create_generic_label( seconds, font_instructions );
	gtk_box_pack_start( GTK_BOX( form ), instructions_label, FALSE, FALSE, 20 );
	gtk_widget_show( instructions_label );

	GtkWidget* button = NULL;
	hbox = gtk_hbox_new( FALSE, 0 );
	for( int i = 0; i < 2; i++ )
	{
		if( i == 0 )
		{
			button = create_generic_button( _("No") );
			g_signal_connect( button, "clicked", G_CALLBACK( back_callback ), NULL );
			gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
		}
		else
		{
			button = create_generic_button( _("Yes") );
			g_signal_connect( button, "clicked", G_CALLBACK( next_callback ), NULL );
			gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
		}
		gtk_widget_show( button );
	}
	gtk_box_pack_end( GTK_BOX( form ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	return form;
}

GtkWidget* create_digital_keyboard_form( const char* hint_text )
{
	GtkWidget* form = gtk_vbox_new( FALSE, 10 );
	GtkWidget* hbox = create_lang_panel();

	gtk_box_pack_start( GTK_BOX( form ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	hbox = gtk_hbox_new( FALSE, 0 );
	// gtk_container_set_border_width( GTK_CONTAINER( hbox ), 10 );

	hint_labels[0] = gtk_label_new( hint_text );
	gtk_widget_modify_font( hint_labels[0], font_labels );
	gtk_widget_modify_fg( hint_labels[0], GTK_STATE_NORMAL, &black_color );
	gtk_box_pack_start( GTK_BOX( hbox ), hint_labels[0], FALSE, FALSE, 20 );
	gtk_widget_show( hint_labels[0] );

	// hbox = gtk_hbox_new( FALSE, 10 );

	value_labels[0] = gtk_label_new( "" );
	gtk_widget_modify_font( value_labels[0], font_instructions );
	gtk_widget_modify_fg( value_labels[0], GTK_STATE_NORMAL, &black_color );
	gtk_box_pack_start( GTK_BOX( hbox ), value_labels[0], FALSE, FALSE, 20 );
	gtk_widget_show( value_labels[0] );

	GtkWidget* button = gtk_button_new();
	gtk_button_set_image( GTK_BUTTON( button ), gtk_image_new_from_pixbuf( load_image_from_resource( "/delete.png" ) ) );
	gtk_widget_modify_bg( button, GTK_STATE_NORMAL, &onay_color );
	gtk_widget_modify_bg( button, GTK_STATE_ACTIVE, &onay_color );
	gtk_widget_modify_bg( button, GTK_STATE_PRELIGHT, &onay_color );
	gtk_widget_modify_bg( button, GTK_STATE_INSENSITIVE, &onay_color );
	gtk_widget_set_can_focus( button, FALSE );
	gtk_widget_set_size_request( button, NUM_BTN_SIZE, NUM_BTN_SIZE );
	g_signal_connect( button, "clicked", G_CALLBACK( key_callback ), GINT_TO_POINTER( -1 ) );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 20 );
	gtk_widget_show( button );

	gtk_box_pack_start( GTK_BOX( form ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	char but[1];
	GtkWidget* label = NULL;
	hbox = gtk_hbox_new( TRUE, 0 );
	// gtk_widget_set_size_request( hbox, NUM_BTN_SIZE * 8, NUM_BTN_SIZE );
	// gtk_container_set_border_width( GTK_CONTAINER( hbox ), 10 );

	for( guint i = 1; i <= 10; i++ )
	{
		sprintf( but, "%u", i % 10 );
		button = gtk_button_new();
		gtk_widget_modify_bg( button, GTK_STATE_NORMAL, &onay_color );
		gtk_widget_modify_bg( button, GTK_STATE_ACTIVE, &onay_color );
		gtk_widget_modify_bg( button, GTK_STATE_PRELIGHT, &onay_color );
		gtk_widget_modify_bg( button, GTK_STATE_INSENSITIVE, &onay_color );
		gtk_widget_set_can_focus( button, FALSE );
		gtk_widget_set_size_request( button, NUM_BTN_SIZE, NUM_BTN_SIZE );

		label = gtk_label_new( but );
		gtk_widget_modify_fg( label, GTK_STATE_NORMAL, &black_color );
		gtk_widget_modify_fg( label, GTK_STATE_PRELIGHT, &black_color );
		gtk_widget_modify_font( label, font_buttons );
		gtk_container_add( GTK_CONTAINER( button ), label );
		gtk_widget_show( label );

		g_signal_connect( button, "clicked", G_CALLBACK( key_callback ), GINT_TO_POINTER( i % 10 ) );
		gtk_box_pack_start( GTK_BOX( hbox ), button, TRUE, TRUE, 20 );
		gtk_widget_show( button );

		if( i == 5 || i == 10 )
		{
			gtk_box_pack_start( GTK_BOX( form ), hbox, FALSE, FALSE, 0 );
			gtk_widget_show( hbox );
			hbox = gtk_hbox_new( TRUE, 0 );
		}
	}
	hbox = gtk_hbox_new( FALSE, 0 );

	back_button = create_generic_button( _("Exit") );
	g_signal_connect( back_button, "clicked", G_CALLBACK( back_callback ), NULL );
	gtk_box_pack_start( GTK_BOX( hbox ), back_button, FALSE, FALSE, 0 );
	gtk_widget_show( back_button );

	next_button = create_generic_button( _("Next") );
	gtk_box_pack_end( GTK_BOX( hbox ), next_button, FALSE, FALSE, 0 );
	g_signal_connect( next_button, "clicked", G_CALLBACK( next_callback ), NULL );
	// gtk_widget_show( next_button );

	gtk_box_pack_end( GTK_BOX( form ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	return form;
}

GtkWidget* create_manual_input_form( gboolean reload_ui )
{
	GtkWidget* form = create_digital_keyboard_form( _("CardNumber") );
	if( !reload_ui )
	{
		bzero( input_text, 32 );
		reset_counter_timer();
	}
	else if( strlen( input_text ) > 0 )
	{
		gtk_label_set_text( GTK_LABEL( value_labels[0] ), input_text );
		gtk_widget_show( next_button );
	}
	return form;
}

GtkWidget* create_pin_code_form( gboolean reload_ui )
{
	GtkWidget* form = create_digital_keyboard_form( _("PinCode") );
	int len = strlen( input_text );
	if( !reload_ui )
	{
		bzero( input_text, 32 );
		reset_counter_timer();
	}
	else if( len > 0 )
	{
		char pin[10];
		bzero( pin, 10 );
		for( int i = 0; i < len; i++ )
			strcat( pin, "*" );
		gtk_label_set_text( GTK_LABEL( value_labels[0] ), pin );
		gtk_widget_show( next_button );
	}
	return form;
}

void display_more_time_form( int counter )
{
	if( curr_screen == SCREEN_MORE_TIME )
	{
		char seconds[8];
		sprintf( seconds, "%d", counter );
		gtk_label_set_text( GTK_LABEL( instructions_label ), seconds );
	}
	else
	{
		remove_last_form();
		curr_form = create_more_time_form( counter );
		gtk_container_add( GTK_CONTAINER( main_window ), curr_form );
		gtk_widget_show( curr_form );
		gtk_widget_show( main_window );

		last_screen = curr_screen;
		curr_screen = SCREEN_MORE_TIME;
	}
}

void reload_form( enum APP_SCREEN screen, gboolean reload_ui )
{
	GtkWidget* new_form = NULL;

	switch( screen )
	{
	case SCREEN_LOADING:
		new_form = create_loading_form();
		break;

	case SCREEN_INTRO:
		stop_playback();
		// init_video();
		new_form = create_intro_form();
		break;

	case SCREEN_HOME:
		if( operating() )
			new_form = create_home_form( reload_ui );
		else
		{
			new_form = create_out_of_service_form( reload_ui );
			screen = SCREEN_OUT_OF_SERVICE;
		}
		break;

	case SCREEN_NO_CARDS:
		new_form = create_no_cards_form( reload_ui );
		break;

	case SCREEN_PASS_INTRO:
		new_form = create_pass_intro_form( reload_ui );
		break;

	case SCREEN_PASS_FIRST:
		new_form = create_pass_first_form( reload_ui );
		break;

	case SCREEN_PASS_ALREADY_ACTIVE:
		new_form = create_pass_already_active_form( reload_ui );
		break;

	case SCREEN_PASS_SUFFICIENT_BALANCE:
		new_form = create_pass_sufficient_balance_form( reload_ui );
		break;

	case SCREEN_PASS_PARTIAL_AMOUNT:
		new_form = create_pass_partial_amount_form( reload_ui );
		break;

	case SCREEN_PASS_SUFFICIENT_AMOUNT:
		new_form = create_pass_sufficient_amount_form( reload_ui );
		break;

	case SCREEN_PASS_INSUFFICIENT_AMOUNT:
		new_form = create_pass_insufficient_amount_form( reload_ui );
		break;

	case SCREEN_PASS_ACTIVE:
		new_form = create_pass_active_form( reload_ui );
		break;

	case SCREEN_PASS_ACTIVE_NEXT_MONTH:
		new_form = create_pass_active_next_month_form( reload_ui );
		break;

	case SCREEN_PASS_ACTIVE_WHEN_CONNECTED:
		new_form = create_pass_active_when_connected_form( reload_ui );
		break;

	case SCREEN_CARD_INTRO:
		new_form = create_card_intro_form( reload_ui );
		break;

	case SCREEN_CARD_PARTIAL_AMOUNT:
		new_form = create_card_partial_amount_form( reload_ui );
		break;

	case SCREEN_CARD_SUFFICIENT_AMOUNT:
		new_form = create_card_sufficient_amount_form( reload_ui );
		break;

	case SCREEN_CARD_INSUFFICIENT_AMOUNT:
		new_form = create_card_insufficient_amount_form( reload_ui );
		break;

	case SCREEN_CARD_FREE:
		new_form = create_card_free_form( reload_ui );
		break;

	case SCREEN_CARD_PRE_SCAN:
		new_form = create_card_pre_scan_form( reload_ui );
		break;

	case SCREEN_CARD_SCAN:
		new_form = create_card_scan_form( reload_ui );
		break;

	case SCREEN_BALANCE_INTRO:
		new_form = create_balance_intro_form( reload_ui );
		break;

	case SCREEN_BALANCE_REFILL:
		new_form = create_balance_refill_form( reload_ui );
		break;

	case SCREEN_BALANCE_REFILLED:
		new_form = create_balance_refilled_form( reload_ui );
		break;

	case SCREEN_BALANCE_REFILLED_WHEN_CONNECTED:
		new_form = create_balance_refilled_when_connected_form( reload_ui );
		break;

	case SCREEN_CARD_NOT_FOUND:
		new_form = create_card_not_found_form( reload_ui );
		break;

	case SCREEN_CARD_BOUGHT:
		new_form = create_card_bought_form( reload_ui );
		break;

	case SCREEN_OUT_OF_SERVICE:
		new_form = create_out_of_service_form( reload_ui );
		break;

	case SCREEN_MANUAL_INPUT:
		new_form = create_manual_input_form( reload_ui );
		break;

	case SCREEN_PIN_CODE:
		new_form = create_pin_code_form( reload_ui );
		break;

	case SCREEN_MORE_TIME:
		trace( "Calling: display_form( SCREEN_MORE_TIME ). Should call display_more_time_form( counter ) instead" );
		return;

	default:

		break;
	};

	remove_last_form();
	curr_form = new_form;
	gtk_container_add( GTK_CONTAINER( main_window ), curr_form );
	gtk_widget_show( curr_form );
	gtk_widget_show( main_window );

	if( curr_screen != screen )
	{
		last_screen = curr_screen;
		curr_screen = screen;
	}

	if( screen == SCREEN_INTRO )
	{
		gtk_widget_realize( curr_form );
		start_playback();
	}
}

void display_form( enum APP_SCREEN screen )
{
	trace( "display_form( %s )\n", APP_SCREEN_NAMES[(int)screen] );
	reload_form( screen, FALSE );
}

void refresh_form()
{
	reload_form( curr_screen, TRUE );
}
// -------------- ui --------------------------------------------------------------------<

void turn_on_spi()
{
	FILE* file = fopen( "/sys/class/gpio/gpio100/value", "w" );
	if( file != NULL )
	{
		fprintf( file, "0" );
		fclose( file );
	}
}

void turn_off_spi()
{
	FILE* file = fopen( "/sys/class/gpio/gpio100/value", "w" );
	if( file != NULL )
	{
		fprintf( file, "1" );
		fclose( file );
	}
}

uint8_t spi_send_command( int fd, uint16_t delay, uint32_t speed, uint8_t bits, uint8_t cmd )
{
	uint8_t tx[1];
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };

	tx[0] = cmd;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	if( ioctl( fd, SPI_IOC_MESSAGE( 1 ), &tr ) < 0 )
		trace( "can't send spi message" );

	tx[0] = 0x00;
	tr.tx_buf = (unsigned long)tx;

	if( ioctl( fd, SPI_IOC_MESSAGE( 1 ), &tr ) < 0 )
		trace( "can't send spi message" );

	return rx[0];

	/*
	struct spi_ioc_transfer xfer;
	memset( &xfer, 0, sizeof( xfer ) );
	char tx[2];
	char rx[2];
	tx[0] = cmd;
	tx[1] = 0x00;
	xfer.tx_buf = (unsigned long)tx;
	xfer.rx_buf = (unsigned long)rx;
	xfer.len = 2;
	xfer.speed_hz = 500000;
	xfer.cs_change = 1;
	xfer.bits_per_word = 8;
	if( ioctl( fd, SPI_IOC_MESSAGE( 1 ), &xfer ) < 0 )
		trace( "can't send spi message" );

	return rx[0];
	*/
}

gpointer spi_thread_func( gpointer data )
{
	const char* device = "/dev/spidev3.0";
	int fd = open( device, O_RDWR );
	if( fd >= 0 )
	{
		uint8_t mode = 0, bits = 8;
		uint32_t speed = 62500; // 500000; 2000000
		uint16_t delay = 10;

		int ret = ioctl( fd, SPI_IOC_WR_MODE, &mode );
		if( ret > -1 )
			ret = ioctl( fd, SPI_IOC_WR_BITS_PER_WORD, &bits );
		else
			trace( "can't set spi write mode" );

		if( ret > -1 )
			ret = ioctl( fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed );
		else
			trace( "can't set write bits per word" );

		if( ret > -1 )
			ret = ioctl( fd, SPI_IOC_RD_MODE, &mode );
		else
			trace( "can't set max write speed hz" );

		if( ret > -1 )
			ret = ioctl( fd, SPI_IOC_RD_BITS_PER_WORD, &bits );
		else
			trace( "can't set spi read mode" );

		if( ret > -1 )
			ret = ioctl( fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed );
		else
			trace( "can't set read bits per word" );

		// int lsb_setting = 1;
		// ioctl( fd, SPI_IOC_WR_LSB_FIRST, &lsb_setting );

		if( ret > -1 )
		{
		    int sleep_microseconds = 200 * 1000;
		    int check_microseconds = 20 * 1000;

/*
		    struct timeval start, end;
		    gettimeofday( &start, NULL );
*/

			turn_on_spi();
			uint8_t status = 0x00;

			while( TRUE )
			{
				if( unlock_requested > UNLOCK_NONE )
				{
					trace( "Sending Wakeup call" );
					trace( "%02X", spi_send_command( fd, delay, speed, bits, 0xAA ) );
					trace( "Sending Unlock call" );

					if( unlock_requested == UNLOCK_FINANCIAL )
						status = spi_send_command( fd, delay, speed, bits, 0xE1 );
					else
						status = spi_send_command( fd, delay, speed, bits, 0xE0 );
					trace( "%02X", status );

					if( status != 0xff )
					{
						// display_form( SCREEN_PASS_GRANTED );
					}

					for( int i = 0; i < 400; i++ )
					{
						// trace( "Check State call" );
						if( unlock_requested == UNLOCK_FINANCIAL )
							status = spi_send_command( fd, delay, speed, bits, 0xF1 );
						else
							status = spi_send_command( fd, delay, speed, bits, 0xF0 );
						// trace( "%02X", status );

						// if( status == 0x88 )
						if( status != 0x30 )
							break;

						if( i % 50 == 0 )
							issue_sound_signal = 1;

						usleep( check_microseconds );
					}

					trace( "Check complete!" );
					unlock_requested = UNLOCK_NONE;
				}

				usleep( sleep_microseconds );
			}

			turn_off_spi();
		}
		else
			trace( "can't set max read speed hz" );

		close( fd );
	}
	else
		trace( "can't open SPI device" );

	trace( "leaving SPI thread" );

	return NULL;
}



int read_printer_paper_status( int fd )
{
	int pstatus = PAPER_UNKNOWN; // not read
	char c;
	if( read( fd, &c, 1 ) > 0 )
	{
		if( ( c & ( 1 << 6 ) ) > 0 && ( c & ( 1 << 5 ) ) > 0 )
			pstatus = PAPER_EMPTY;
		else if( ( c & ( 1 << 3 ) ) > 0 && ( c & ( 1 << 2 ) ) > 0 )
			pstatus = PAPER_LOW;
		else
			pstatus = PAPER_FULL;
	}

	return pstatus;
}

int read_printer_status( int fd )
{
	int pstatus = PRINTER_NOT_READY;
	char c;
	if( read( fd, &c, 1 ) > 0 )
	{
		trace( "PRINTER STATUS: %d", (int)c );

		if( ( c & ( 1 << 3 ) ) > 0 )
			pstatus = PRINTER_OFFLINE;
		else
			pstatus = PRINTER_OK;
	}
	return pstatus;
}


int read_printer_error( int fd )
{
	int pstatus = PRINTER_NOT_READY;
	char c;
	if( read( fd, &c, 1 ) > 0 )
		pstatus = c;

	return pstatus;
}

gpointer cheque_printer_thread_func( gpointer data )
{
    int sleep_microseconds = 200 * 1000;
    int connect_attempt_seconds = 30;
    struct termios tty;

    while( TRUE )
    {
    	trace( "Init printer..." );
		system( "stty 115200 -F /dev/ttyUSB0 raw -echo" );
		int fd = open( "/dev/ttyUSB0", O_RDWR | O_NOCTTY /*| O_NONBLOCK | O_NDELAY*/ );
		if( fd >= 0 )
		{
			tcgetattr( fd, &tty );

			tty.c_cflag = ( tty.c_cflag & ~CSIZE ) | CS8;
			tty.c_cflag |= B115200;
			tty.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG );
			tty.c_iflag &= ~( IXON | IXOFF | IXANY ); // shut off xon/xoff ctrl
			tty.c_oflag &= ~OPOST; // raw data output

			tty.c_cc[VMIN] = 0;
			tty.c_cc[VTIME] = 20; // 2 seconds: VTIME takes 1/10-th of a second

			if( tcsetattr( fd, TCSANOW, &tty ) != 0 )
			{
				fprintf( stderr, "Cheque Printer error %d from tcsetattr", errno );
				close( fd );
				return NULL;
			}

			char reset[] = { 0x1b, 0x40 };
			char cut[] = { 0x1b, 0x69 };
			// char recover[] = { 0x10, 0x05, 0x02 };
			char get_printer_status[] = { 0x10, 0x04, 0x01 };
			char get_paper_status[] = { 0x10, 0x04, 0x04 };
			// char test[] = { 0x1d, 0x28, 0x41, 0x02, 0x00, 0x00, 0x02 };

			write( fd, get_paper_status, sizeof( get_printer_status ) );
			usleep( sleep_microseconds );
			int prnt_status = read_printer_status( fd );

			int prnt_status_read_fails = 0;
			enum PRINTER_PAPER_STATUS ppr_status = PAPER_UNKNOWN;
			if( prnt_status == PRINTER_OK )
			{
				printer_status = prnt_status;

				write( fd, get_paper_status, sizeof( get_paper_status ) );
				usleep( sleep_microseconds );
				ppr_status = read_printer_paper_status( fd );
				if( ppr_status > PAPER_UNKNOWN )
				{
					paper_status = ppr_status;
					if( paper_status != PAPER_FULL )
						status_report_requested = TRUE;
				}
			}
			else
			{
				printer_status = prnt_status;
				prnt_status_read_fails ++;
			}

			struct timeval start, end;
			gettimeofday( &start, NULL );

			// trace( "Printer thread");

			while( TRUE )
			{
				if( printer_status == PRINTER_OK )
				{
					if( print_cheque_requested )
					{
						write( fd, reset, sizeof( reset ) );
						//usleep( sleep_microseconds );
						//tcflush( fd, TCIOFLUSH );
						print_cheque_sync( fd );
						//usleep( sleep_microseconds );
						//tcflush( fd, TCIOFLUSH );
						write( fd, cut, sizeof( cut ) );
						//usleep( sleep_microseconds );
						//tcflush( fd, TCIOFLUSH );
						write( fd, get_paper_status, sizeof( get_paper_status ) );
						usleep( sleep_microseconds );
						ppr_status = read_printer_paper_status( fd );
						if( ppr_status > PAPER_UNKNOWN )
						{
							if( paper_status != ppr_status )
							{
								paper_status = ppr_status;
								status_report_requested = TRUE;
							}
						}
						trace( "Paper status: %d", ppr_status );
						print_cheque_requested = FALSE;
					}
				}
				else
				{
					gettimeofday( &end, NULL );
					if( end.tv_sec - start.tv_sec >= PRINTER_STATUS_SECONDS )
					{
						gettimeofday( &start, NULL );
						write( fd, get_printer_status, sizeof( get_printer_status ) );
						usleep( sleep_microseconds );
						int prnt_status = read_printer_status( fd );
						if( prnt_status == PRINTER_OK )
						{
							prnt_status_read_fails = 0;
							printer_status = prnt_status;
							status_report_requested = TRUE;
						}
						else
						{
							printer_status = prnt_status;
							prnt_status_read_fails ++;

							/*
							char get_printer_offline[] = { 0x10, 0x04, 0x02 };
							write( fd, get_printer_offline, sizeof( get_printer_offline ) );
							usleep( sleep_microseconds );
							int prnt_offline = read_printer_error( fd );
							trace( "OFFLINE STATUS: %d", prnt_offline );

							char get_printer_error[] = { 0x10, 0x04, 0x03 };
							write( fd, get_printer_error, sizeof( get_printer_error ) );
							usleep( sleep_microseconds );
							int prnt_error = read_printer_error( fd );
							trace( "PRINTER ERROR: %d", prnt_error );
							*/

							if( prnt_status_read_fails > 1 )
							{
								prnt_status_read_fails = 0;

								write( fd, reset, sizeof( reset ) );
								usleep( sleep_microseconds );
								trace( "Printer reset" );
								break;

								/*
								if( prnt_status == PRINTER_OFFLINE )
								{
									write( fd, recover, sizeof( recover ) );
									usleep( sleep_microseconds );
									trace( "Printer recover" );
								}
								else
								{
									write( fd, reset, sizeof( reset ) );
									// write( fd, test, sizeof( test ) );
									usleep( sleep_microseconds );
									trace( "Printer reset" );
									break;
								}
								*/
							}
						}
					}
				}
				usleep( sleep_microseconds );
			}
			close( fd );
		}
		else
		{
			printer_status = PRINTER_NOT_READY;
			sleep( connect_attempt_seconds );
		}
    }
	return NULL;
}



gpointer card_dispenser_thread_func( gpointer data )
{
	system( "stty 9600 -F /dev/ttymxc3 raw -echo" );
    int delay_microseconds = 50 * 100;

	struct termios tty;
	int fd = open( "/dev/ttymxc3", O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY );
	if( fd >= 0 )
	{
		tcgetattr( fd, &tty );

		tty.c_cflag = ( tty.c_cflag & ~CSIZE ) | CS8;
		tty.c_cflag |= B9600;
		tty.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG );
		tty.c_iflag &= ~( IXON | IXOFF | IXANY ); // shut off xon/xoff ctrl

		if( tcsetattr( fd, TCSANOW, &tty ) != 0 )
		{
			trace( "Card Dispenser error %d from tcsetattr", errno );
			dispenser_status = DISPENSER_EXECUTION_ERROR;
			return NULL;
		}

		char dispense[] = { 0x02, 0x31, 0x35, 0x44, 0x43, 0x03, 0x02 };
		char status[]   = { 0x02, 0x31, 0x35, 0x41, 0x50, 0x03, 0x14 };
		char reset[]    = { 0x02, 0x31, 0x35, 0x52, 0x53, 0x03, 0x04 };

		char buf[24], c;
		int i = 0, n = 0, bufsize = sizeof( buf );
		bzero( buf, bufsize );

		write( fd, status, sizeof( status ) );
		usleep( delay_microseconds );

	    struct timeval start, end;
	    gettimeofday( &start, NULL );

	    gboolean dispensing = FALSE, reset_requested = FALSE;
	    // int dispenser_error = 0;

		do
		{
			n = read( fd, &c, 1 );
			if( n > 0 )
			{
				// printf( "%02X", c );
				if( i < bufsize )
				{
					buf[i++] = c;
				}
				else
				{
					i = 0;
					bzero( buf, bufsize );
					trace( "Card Dispenser buffer overflow!" );
				}
			}
			else if( reset_requested )
			{
				reset_requested = FALSE;
				trace( "Card dispenser reset" );
				write( fd, reset, sizeof( reset ) );
			}
			else if( card_dispense_requested && !dispensing )
			{
				card_dispense_requested = FALSE;

				dispensing = TRUE;
				dispense_attempts ++;
				trace( "Card dispense requested" );
				write( fd, dispense, sizeof( dispense ) );
			}
			else
			{
				if( i > 0 )
				{
					// print_hex( buf, i );

					if( i == 3 )						// just Ack or Nack
					{
						if( buf[0] == 0x06 ) 			// Ack
						{
							// trace( "Ack" );
						}
						else if( buf[0] == 0x15 && dispensing ) // Nack
						{
							card_dispense_requested = TRUE;
							trace( "Dispense Nack" );
						}
					}
					else if( i > 7 && buf[0] == 0x06 )	// Ack + Status
					{
						if( dispenser_status != buf[6] || stacker_status != buf[7] )
							trace( "Dispenser: %02X Stacker: %02X; Dispensing: %d", buf[6], buf[7], dispensing );

						gboolean report_requested = FALSE;
						if( dispenser_status != buf[6] )
						{
							if( dispenser_status == DISPENSER_DISPENSING_ERROR || dispenser_status == DISPENSER_EXECUTION_ERROR || dispenser_status == DISPENSER_NOT_READY || buf[6] == DISPENSER_DISPENSING_ERROR || buf[6] == DISPENSER_EXECUTION_ERROR )
							{
								trace( "Dispenser status changed" );
								report_requested = TRUE;
							}
							dispenser_status = buf[6];
						}

						if( stacker_status != buf[7] )
						{
							report_requested = TRUE;
							stacker_status = buf[7];
							// trace( "Dispenser: %02X Stacker: %02X", dispenser_status, stacker_status );
						}

						if( report_requested )
							status_report_requested = TRUE;


						if( dispenser_status == DISPENSER_DISPENSING_ERROR || dispenser_status == DISPENSER_EXECUTION_ERROR || dispenser_status == DISPENSER_UNKNOWN_ERROR )
						{
							if( dispensing )
							{
								dispensing = FALSE;

								trace( "Failed to dispense card!" );
								if( dispense_attempts < 2 )
									card_dispense_requested = TRUE;
							}

							/*
							dispenser_error ++;

							if( dispensing )
							{
								if( transaction.amount > 0 ) // if transaction is still active - try again
									card_dispense_requested = TRUE;
								trace( "Failed to dispense card!" );
								dispensing = FALSE;
							}

							if( dispenser_error % 2 == 0 )
								reset_requested = TRUE;
							*/
						}
						else if( dispenser_status == DISPENSER_READY || dispenser_status == DISPENSER_DISPENSING_SUCCESS )
						{
							if( dispensing )
							{
								dispensing = FALSE;
								card_dispensed();
								dispense_attempts = 0;
							}

							/*
							if( dispenser_error > 0 )
								dispenser_error = 0;

							if( dispensing )
							{
								card_dispensed();
								dispensing = FALSE;
							}
							*/
						}
					}

					i = 0;
					bzero( buf, bufsize );
				}

				gettimeofday( &end, NULL );
				if( ( end.tv_sec - start.tv_sec ) >= DISPENSER_STATUS_SECONDS )
				{
					gettimeofday( &start, NULL );
					write( fd, status, sizeof( status ) );
				}
				usleep( delay_microseconds );
			}
		}
		while( TRUE );
		close( fd );
	}
	else
	{
		dispenser_status = DISPENSER_EXECUTION_ERROR;
		trace( "Failed to open Card Dispenser port\n" );
	}

	return NULL;
}

gpointer bill_acceptor_thread_func( gpointer data )
{
	// cash_retrieval_enabled = TRUE;

	system( "stty 9600 -F /dev/ttymxc4 raw -echo" );
    int delay_microseconds = 50 * 1000;

	struct termios tty;

	int fd = open( "/dev/ttymxc4", O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY );
	if( fd >= 0 )
	{
		tcgetattr( fd, &tty );

		tty.c_cflag = ( tty.c_cflag & ~CSIZE ) | CS8;
		tty.c_cflag |= B9600;
		tty.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG );
		tty.c_iflag &= ~( IXON | IXOFF | IXANY ); // shut off xon/xoff ctrl

		if( tcsetattr( fd, TCSANOW, &tty ) != 0 )
		{
			fprintf( stderr, "Bill Acceptor error %d from tcsetattr", errno );
			bill_acceptor_status = FAILURE;
			return NULL;
		}

		char reset[] = { 0x02, 0x03, 0x06, 0x30, 0x41, 0xB3 };
		char poll[]  = { 0x02, 0x03, 0x06, 0x33, 0xDA, 0x81 };
		char ack[]   = { 0x02, 0x03, 0x06, 0x00, 0xC2, 0x82 };
		char nack[]  = { 0x02, 0x03, 0x06, 0xFF, 0xBA, 0x8D };
		char enbl[]	 = { 0x02, 0x03, 0x0C, 0x34, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xB5, 0xC1 };
		// char status[] = { 0x02, 0x03, 0x06, 0x31, 0xC8, 0xA2 };
		char bill = 0;
		// char table[] = { 0x02, 0x03, 0x06, 0x41, 0x4F, 0xD1 };
		// char model[] = { 0x02, 0x03, 0x06, 0x37, 0xFE, 0xC7 };
		// char sec[]   = { 0x02, 0x03, 0x09, 0x32, 0x00, 0x00, 0x00, 0x26, 0x1F };


		gboolean wait_answ = TRUE;
		gboolean enabled = FALSE;
		char buf[64], c, psc = bill_acceptor_status;
		int i = 0, n = 0, bufsize = sizeof( buf ), len = 0;
		bzero( buf, len );

		// trace( "reset" );
		write( fd, reset, sizeof( reset ) );
		// trace( "poll" );
		usleep( delay_microseconds );
		write( fd, poll, sizeof( poll ) );
		usleep( delay_microseconds );
		// trace( "status" );
		// write( fd, status, sizeof( status ) );

		/*
	    struct timeval start, end;
	    gettimeofday( &start, NULL );
	    */


		do
		{
			n = read( fd, &c, 1 );
			if( n > 0 )
			{
				// printf( "%02X", c );
				if( i < bufsize )
				{
					if( i == 0 && c == 0x02 )	// start of packet
					{
						buf[i++] = c;
					}
					else if( i > 0 )
					{
						if( len > 0 )
						{
							if( i < len )
							{
								if( i == len - 1 )	// end of packet
								{
									buf[i] = c;
									buf[i + 1] = '\0';
									// print_hex( buf, len );

									if( check_crc( buf, len ) )
									{
										write( fd, ack, sizeof( ack ) );
										// trace( "Ack" );

										if( bill_acceptor_status != psc )
										{
											gboolean report_requested = FALSE;
											// print_hex( buf, len );
											if( bill_acceptor_status == BILL_STACKED )
											{
												if( bill < 6 )
												{
													// printf( "Bill accepted: %d \n", BILL_NOMINATIONS[(int)bill] );
													trace( "Bill accepted: %d \n", BILL_NOMINATIONS[(int)bill] );

													gdk_threads_enter();
													// deposit_amount( (int)bill );
													deposit_amount( BILL_NOMINATIONS[(int)bill] );
													gdk_threads_leave();
												}
												else
												{
													trace( "Error accepting bill - index out of range!" );
													trace( "Bill accepted index: %2X \n", bill );
													bill_acceptor_status = FAILURE;
													report_requested = TRUE;
												}
											}
											else if( bill_acceptor_status >= REJECTING && bill_acceptor_status < BILL_STACKED )
											{
												trace( "Bill acceptor status: %2X", bill_acceptor_status );
												if( ( bill_acceptor_status >= DROP_FULL && bill_acceptor_status <= CASSETTE_JAMMED ) || bill_acceptor_status == FAILURE )
													report_requested = TRUE;
											}

											if( report_requested || psc == BILL_ACCEPTOR_UNKNOWN )
												status_report_requested = TRUE;
										}
									}
									else
									{
										write( fd, nack, sizeof( nack ) );
										trace( "Nack" );
									}

									i = 0;
									len = 0;
									bzero( buf, bufsize );
									wait_answ = FALSE;
									psc = bill_acceptor_status;

									usleep( delay_microseconds );
								}
								else
								{
									if( i == 3 )
										bill_acceptor_status = c;		// status code
									else if( i == 4 && bill_acceptor_status == BILL_STACKED )
										bill = c;

									buf[i++] = c;
								}
							}
							else
							{
								trace( "Not supposed to be here!" );
								i = 0;
								len = 0;
								bzero( buf, bufsize );
							}
						}
						else
						{
							if( i == 2 )	// length byte
							{
								// printf( "Got length byte: %02X\n", c );
								len = c;
							}
							buf[i++] = c;
						}
					}
				}
				else
				{
					i = 0;
					len = 0;
					bzero( buf, bufsize );
					trace( "Bill Acceptor buffer overflow!" );
				}
			}
			else if( cash_retrieval_enabled )
			{
				if( !wait_answ )
				{
					if( !enabled )
					{
						enabled = TRUE;
						// trace( "Enable request" );
						write( fd, enbl, sizeof( enbl ) );
					}
					else
					{
						// trace( "Poll request" );
						write( fd, poll, sizeof( poll ) );
					}
					usleep( delay_microseconds );
					wait_answ = TRUE;
				}
			}
			else
			{
				if( wait_answ || enabled )
				{
					wait_answ = FALSE;
					enabled = FALSE;
				}

				/*
				gettimeofday( &end, NULL );
				if( end.tv_sec - start.tv_sec >= 30 )
				{
					i = 0;
					len = 0;
					bzero( buf, bufsize );
					gettimeofday( &start, NULL );
					trace( "Status request" );
					write( fd, status, sizeof( status ) );
				}
				*/

				usleep( delay_microseconds );
			}
		}
		while( TRUE );
		close( fd );
	}
	else
	{
		bill_acceptor_status = FAILURE;
		trace( "Failed to open Bill Acceptor port\n" );
	}

	return NULL;
}



void card_read( const char* pan, const char* uid )
{
	if( !card.initialized )
	{
		card.type = get_card_type( pan );
		if( card.type == 0 )
		{
			if( curr_screen == SCREEN_HOME || curr_screen == SCREEN_INTRO || curr_screen == SCREEN_CARD_SCAN || curr_screen == SCREEN_PASS_INTRO )
			{
				if( terminal.connected )
				{
					strcpy( card.pan, pan );
					strcpy( card.uid, uid );
					card.initialized = TRUE;

					gdk_threads_enter();
					stop_playback();
					display_form( SCREEN_LOADING );
					gdk_threads_leave();
					card_activated();
				}
				else if( curr_state == STATE_IDLE )
				{
					gdk_threads_enter();
					stop_playback();
					display_form( SCREEN_OUT_OF_SERVICE );
					gdk_threads_leave();
				}
			}
		}
		else if( card.type == 1 || card.type == 2 )
		{
			if( curr_screen == SCREEN_HOME || curr_screen == SCREEN_INTRO || curr_screen == SCREEN_PIN_CODE || curr_screen == SCREEN_OUT_OF_SERVICE )
			{
				if( get_trials_count( pan ) < 3 )
				{
					strcpy( card.pan, pan );
					strcpy( card.uid, uid );
					card.initialized = TRUE;

					if( card.type == 1 )
						curr_state = STATE_COLLECT;
					else
						curr_state = STATE_MAINTENANCE;
					gdk_threads_enter();
					stop_playback();
					display_form( SCREEN_PIN_CODE );
					gdk_threads_leave();
				}
			}
		}
		issue_sound_signal = 1;
	}
}

void send_ack( int fd )
{
	char ack_pre_11[] = { 0x06, 0xb1 };
	write( fd, ack_pre_11, sizeof( ack_pre_11 ) );
	char ack[] = { 0x60, 0x00, 0x06, 0xb1, 0xd7 };
	write( fd, ack, sizeof( ack ) );
}

void send_nack( int fd )
{
	char nack_pre_11[] = { 0x06, 0xb3 };
	write( fd, nack_pre_11, sizeof( nack_pre_11 ) );
	char nack[] = { 0x60, 0x00, 0x06, 0xb3, 0xd5 };
	write( fd, nack, sizeof( nack ) );
}

gpointer card_reader_thread_func( gpointer data )
{
	system( "stty 9600 -F /dev/ttymxc1 raw -echo" );
    int delay_microseconds = 250 * 1000;

	struct termios tty;
	int fd = open( "/dev/ttymxc1", O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY );
	if( fd >= 0 )
	{
		tcgetattr( fd, &tty );

		tty.c_cflag = ( tty.c_cflag & ~CSIZE ) | CS8;
		tty.c_cflag |= B9600;
		tty.c_lflag = 0; // no signalling flag, no echo
		tty.c_iflag &= ~( IXON | IXOFF | IXANY ); // shut off xon/xoff ctrl

		if( tcsetattr( fd, TCSANOW, &tty ) != 0 )
		{
			card_reader_status = READER_ERROR;
			trace( "Error initializing CardReader device %d (%s) from tcsetattr", errno, strerror( errno ) );
			return NULL;
		}

		char wakeup[] = { 0xAA };

		write( fd, wakeup, sizeof( wakeup ) );

		char buf[64], hex[3], pan[32], uid[32], c;
		int i = 0, n = 0, bufsize = sizeof( buf ), status_requests = 0;
		bzero( buf, bufsize );

	    struct timeval start, end;
	    gettimeofday( &start, NULL );

		while( TRUE )
		{
			n = read( fd, &c, 1 );
			if( n > 0 )
			{
				// printf( "%02X", c );
				if( i < bufsize )
				{
					if( i == 0 )
					{
						if( c == 0x60 || c == 0x70 )
							buf[i++] = c;
					}
					else
					{
						buf[i++] = c;
					}
				}
				else
				{
					trace( "CardReader buffer overflow!" );
					// print_hex( buf, len );
					i = 0;
					bzero( buf, bufsize );
				}
			}
			else
			{
				if( i > 0 )
				{
					gettimeofday( &start, NULL );

					if( buf[0] == 0x60 )// status
					{
						if( i > 4 )
						{
							// trace( "CardReader status: %02X", buf[3] );

							if( buf[2] == 0x90 )
							{
								if( card_reader_status != READER_OK )
								{
									card_reader_status = READER_OK;
									status_report_requested = TRUE;
								}
							}
							else if( buf[2] == 0x40 )
							{
								if( card_reader_status == READER_OK )
								{
									card_reader_status = buf[3];
									status_report_requested = TRUE;
								}
								else
									card_reader_status = buf[3];
							}

							status_requests = 0;
							i = 0;
							bzero( buf, bufsize );
						}
					}
					else if( buf[0] == 0x70 )
					{
						if( i > 30 )	// data packet
						{
							bzero( pan, sizeof( pan ) );
							bzero( uid, sizeof( uid ) );

							int uid_len = (int)buf[2], offset = 4;

							for( int j = offset; j < (offset + 10); j++ )
							{
								sprintf( hex, "%02X", buf[j] );
								strcat( pan, hex );
							}
							pan[19] = 0;

							offset += 10;	// pan len
							offset += 4;	// start date len + expire date len

							// trace( "Uid len: %d", uid_len );

							offset += (7 - uid_len);
							for( int j = offset; j < (offset + uid_len); j++ )
							{
								sprintf( hex, "%02X", buf[j] );
								strcat( uid, hex );
							}

							/*
							offset += uid_len;	// uid len
							if( strlen( terminal.msam ) == 0 )
							{
								char ascii[2];
								for( int j = offset; j < (offset + 7); j++ )
								{
									sprintf( ascii, "%c", buf[j] );
									strcat( terminal.msam, ascii );
								}
							}

							offset += 7;	// msam len
							if( strlen( terminal.serial ) == 0 )
							{
								char ascii[2];
								for( int j = offset; j < (offset + 7); j++ )
								{
									sprintf( ascii, "%c", buf[j] );
									strcat( terminal.serial, ascii );
								}
							}
							*/

							trace( "Pan: %s; Uid: %s", pan, uid );
							card_read( pan, uid );

							i = 0;
							bzero( buf, bufsize );
							status_requests = 0;
						}
						else
							trace( "Error reading data packet" );
					}
				}
				else if( issue_sound_signal )
				{
					if( issue_sound_signal == 1 )
						send_ack( fd );
					else
						send_nack( fd );
					issue_sound_signal = 0;
				}
				else
				{
					gettimeofday( &end, NULL );
					if( ( end.tv_sec - start.tv_sec ) >= 30 )
					{
						i = 0;
						bzero( buf, bufsize );

						if( status_requests > 0 )
						{
							if( card_reader_status != READER_ERROR )
							{
								trace( "Card Reader unresponsive" );
								card_reader_status = READER_ERROR;
								status_report_requested = TRUE;
							}
							status_requests = 0;
						}

						write( fd, wakeup, sizeof( wakeup ) );
						gettimeofday( &start, NULL );
						status_requests ++;
					}
				}

				usleep( delay_microseconds );
			}
		}
		close( fd );
	}
	else
	{
		card_reader_status = READER_ERROR;
		trace( "Failed to open Card Reader port" );
	}

	trace( "Leaving Card Reader thread" );

	return NULL;
}

/*
gpointer card_reader_thread_func( gpointer data )
{
	system( "stty 9600 -F /dev/ttymxc1 raw -echo" );
    int delay_microseconds = 250 * 1000;

	struct termios tty;
	int fd = open( "/dev/ttymxc1", O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY );
	if( fd >= 0 )
	{
		tcgetattr( fd, &tty );

		tty.c_cflag = ( tty.c_cflag & ~CSIZE ) | CS8;
		tty.c_cflag |= B9600;
		tty.c_lflag = 0; // no signalling flag, no echo
		tty.c_iflag &= ~( IXON | IXOFF | IXANY ); // shut off xon/xoff ctrl

		if( tcsetattr( fd, TCSANOW, &tty ) != 0 )
		{
			card_reader_status = READER_ERROR;
			trace( "Error initializing CardReader device %d (%s) from tcsetattr", errno, strerror( errno ) );
			return NULL;
		}

		char ack[] = { 0x06, 0xb1 };
		char nack[] = { 0x06, 0xb3 };
		char wakeup[] = { 0xAA };

		write( fd, wakeup, sizeof( wakeup ) );

		// trace( "CardReader thread" );
		char buf[64], hex[2], pan[32], uid[16], c;
		int i = 0, n = 0, bufsize = sizeof( buf ), status_requests = 0;
		bzero( buf, bufsize );

	    struct timeval start, end;
	    gettimeofday( &start, NULL );

		while( TRUE )
		{
			n = read( fd, &c, 1 );
			if( n > 0 )
			{
				// printf( "%02X", c );

				if( i < bufsize )
				{
					if( i == 0 )
					{
						if( c == 0x60 || c == 0x70 )
							buf[i++] = c;
					}
					else
					{
						buf[i++] = c;
					}
				}
				else
				{
					trace( "CardReader buffer overflow!" );
					// print_hex( buf, len );
					i = 0;
					bzero( buf, bufsize );
				}
			}
			else
			{
				if( i > 0 )
				{
					if( buf[0] == 0x60 )// status
					{
						if( i > 4 )
						{
							// trace( "CardReader status: %02X", buf[3] );

							if( buf[2] == 0x90 )
							{
								if( card_reader_status > READER_OK )
								{
									card_reader_status = READER_OK;
									status_report_requested = TRUE;
								}
								else
									card_reader_status = READER_OK;
							}
							else if( buf[2] == 0x40 )
							{
								if( card_reader_status == READER_OK )
								{
									card_reader_status = buf[3];
									status_report_requested = TRUE;
								}
								else
									card_reader_status = buf[3];
							}

							status_requests = 0;
							i = 0;
							bzero( buf, bufsize );
						}
					}
					else if( buf[0] == 0x70 )
					{
						if( i > 30 )	// data packet
						{
							bzero( pan, sizeof( pan ) );
							bzero( uid, sizeof( uid ) );

							for( int j = 3; j < 13; j++ )
							{
								sprintf( hex, "%02X", buf[j] );
								strcat( pan, hex );
							}
							pan[19] = 0;

							for( int j = 17; j < 21; j++ )
							{
								sprintf( hex, "%02X", buf[j] );
								strcat( uid, hex );
							}

							if( card_reader_status != READER_OK )
							{
								card_reader_status = READER_OK;
								status_report_requested = TRUE;
							}

							trace( "Pan: %s; Uid: %s", pan, uid );
							card_read( pan, uid );

							i = 0;
							bzero( buf, bufsize );
						}
						else
							print_hex( buf, i );
					}
				}
				else if( issue_sound_signal )
				{
					if( issue_sound_signal == 1 )
						write( fd, ack, sizeof( ack ) );
					else
						write( fd, nack, sizeof( nack ) );
					issue_sound_signal = 0;
				}
				else
				{
					gettimeofday( &end, NULL );
					if( end.tv_sec - start.tv_sec >= 30 )
					{
						i = 0;
						bzero( buf, bufsize );
						if( status_requests >= 3 )
						{
							if( card_reader_status != READER_ERROR )
							{
								trace( "CardReader unresponsive" );
								card_reader_status = READER_ERROR;
								status_report_requested = TRUE;
							}
							status_requests = 0;
						}
						else
							status_requests ++;

						write( fd, wakeup, sizeof( wakeup ) );
						gettimeofday( &start, NULL );
					}
				}

				usleep( delay_microseconds );
			}
		}
		close( fd );
	}
	else
	{
		card_reader_status = READER_ERROR;
		trace( "Failed to open Card Reader port" );
	}

	return NULL;
}
*/


void network_event( enum NETWORK_EVENT event )
{
	// printf( "curr_state: %d\n", curr_state );

	switch( event )
	{
	case NE_CARD_INFO_LOADED:

		// trace( "NE_CARD_INFO_LOADED" );

        if( card.ride_price == 0 )
            display_form( SCREEN_CARD_FREE );
        else
        {
            if( curr_state == STATE_CHECK_REFILL )
            	display_form( SCREEN_BALANCE_INTRO );
            else if( curr_state == STATE_BUY_CARD )
                activate_transaction();
            else if( curr_state == STATE_BUY_PASS )
            {
                time_t now = time( NULL );
                struct tm* tm_info = localtime( &now );

                if( tm_info->tm_mday >= 15 )
                {
                	if( tm_info->tm_mon == 11 )
					{
						tm_info->tm_mon = 0;
						tm_info->tm_year += 1;
					}
					else
						tm_info->tm_mon += 1;
                }

            	if( card_contains_valid_pass( mktime( tm_info ) ) )
            		display_form( SCREEN_PASS_ALREADY_ACTIVE );
            	else
                {
                    if( card.balance >= ( card.pass_price + card.ride_price ) )
                    	display_form( SCREEN_PASS_SUFFICIENT_AMOUNT );
                    else
                    	display_form( SCREEN_PASS_FIRST );
                }
            }
        }
		break;

	case NE_CONNECTION_STATUS_CHANGED:

        if( terminal.connected )
        {
        	// trace( "Connected!" );
        	// trace( "curr_state: %d", curr_state );
        	// trace( "pending transaction: %d", pending_transaction() );
            if( curr_state == STATE_OUT_OF_SERVICE && operating() )
            {
            	curr_state = STATE_IDLE;
                display_form( SCREEN_HOME );
            }
        }
        else
        {
        	trace( "Disconnected!" );
            if( curr_state == STATE_IDLE )
            	display_form( SCREEN_OUT_OF_SERVICE );
        }
		break;

	case NE_CARD_NOT_FOUND:

        if( curr_state == STATE_BUY_CARD )
            activate_transaction();
        else
        	display_form( SCREEN_CARD_NOT_FOUND );
		break;

	case NE_TRANSACTION_COMPLETE:
	case NE_TRANSACTION_FAILED:

        if( curr_state == STATE_CHECK_REFILL )
        {
        	if( transaction.status == TRANSACTION_FISCALIZED || transaction.status == TRANSACTION_COMPLETE )
        		display_form( SCREEN_BALANCE_REFILLED );
        	else
        		display_form( SCREEN_BALANCE_REFILLED_WHEN_CONNECTED );
        }
        else if( curr_state == STATE_BUY_PASS )
        {
            if( ( transaction.amount + card.balance ) >= ( card.pass_price + card.ride_price ) )
            {
            	if( transaction.status == TRANSACTION_FISCALIZED || transaction.status == TRANSACTION_COMPLETE )
            		display_form( SCREEN_PASS_ACTIVE );
            	else
            		display_form( SCREEN_PASS_ACTIVE_WHEN_CONNECTED );

            	/*
                time_t now = time( NULL );
                struct tm* tm_info = localtime( &now );

                if( tm_info->tm_mday < 15 )
                {
                	if( transaction.status == TRANSACTION_FISCALIZED || transaction.status == TRANSACTION_COMPLETE )
                		display_form( SCREEN_PASS_ACTIVE );
                	else
                		display_form( SCREEN_PASS_ACTIVE_WHEN_CONNECTED );
                }
                else
                	display_form( SCREEN_PASS_ACTIVE_NEXT_MONTH );
                */
            }
            else
            	display_form( SCREEN_PASS_INSUFFICIENT_AMOUNT );
        }
        else if( curr_state == STATE_BUY_CARD )
        {
			if( transaction.amount >= transaction.cost )
			{
				if( transaction.status == TRANSACTION_FISCALIZED || transaction.status == TRANSACTION_COMPLETE )
					display_form( SCREEN_CARD_BOUGHT );
				else
					display_form( SCREEN_BALANCE_REFILLED_WHEN_CONNECTED );
			}
			else
				display_form( SCREEN_CARD_INSUFFICIENT_AMOUNT );
        }
        else if( curr_state == STATE_COLLECT )
        {
        	print_cheque();
            clear_transaction();
            display_form( SCREEN_HOME );
        }
        else if( curr_state == STATE_OUT_OF_SERVICE )
        {
            clear_transaction();
            display_form( SCREEN_HOME );
        }
        else if( curr_state == STATE_IDLE )
        {
            clear_transaction();
        }
		break;

	default:
		break;
	};
}

gpointer network_thread_func( gpointer data )
{
    struct timespec delay;
    int sleep_milliseconds = 200;
    delay.tv_sec = sleep_milliseconds / 1000;
    delay.tv_nsec = (sleep_milliseconds % 1000) * 1000000;

    int64_t last_updated_at = 0;
    int TERMINAL_STATUS_INTERVAL_SECONDS = 10; // let some time pass before initial status report so that devices are initialized
    struct timeval trans, start, end;
    gettimeofday( &start, NULL );

    gboolean last_connected = FALSE;

	while( TRUE )
	{
		if( terminal.id == 0 )
		{
			if( get_terminal_id() )
				save_settings();
			else
				sleep( NETWORK_REQUEST_INTERVAL_SECONDS );
		}
		else if( terminal.card_price == 0 )
		{
			if( get_terminal_info() )
			{
				terminal.connected = TRUE;
				TERMINAL_STATUS_INTERVAL_SECONDS = terminal.update_interval * 60;
				gdk_threads_enter();
				network_event( NE_CONNECTION_STATUS_CHANGED );
				gdk_threads_leave();
			}
			else
				sleep( NETWORK_REQUEST_INTERVAL_SECONDS );
		}
		else
		{
			last_connected = terminal.connected;
			gettimeofday( &end, NULL );

			if( last_connected )
			{
				if( card.active )
				{
					card.active = FALSE;
					if( get_card_info() )
					{
						gdk_threads_enter();
						network_event( NE_CARD_INFO_LOADED );
						last_connected = TRUE;
					}
					else
					{
						gdk_threads_enter();
						network_event( NE_CARD_NOT_FOUND );
						if( card.status == CARD_NETWORK_ERROR )
							last_connected = FALSE;
					}
					gdk_threads_leave();
				}

				if( transaction.active )
				{
					if( end.tv_sec - trans.tv_sec >= TRANSACTION_RESEND_INTERVAL_SECONDS )
					{
						if( commit_transaction() )
						{
							gdk_threads_enter();
							network_event( NE_TRANSACTION_COMPLETE );
							last_connected = TRUE;
							clear_transaction();

							trace( "transaction succeeded!" );
						}
						else
						{
							gdk_threads_enter();
							network_event( NE_TRANSACTION_FAILED );

							trace( "transaction failed" );
						}
						gdk_threads_leave();

						gettimeofday( &trans, NULL );
					}
				}
			}

			if( ( ( end.tv_sec - start.tv_sec ) >= TERMINAL_STATUS_INTERVAL_SECONDS ) || status_report_requested )
			{
				if( status_report_requested )
					status_report_requested = FALSE;

				trace( "report_status()" );

			    int64_t updated_at = report_status();
				if( updated_at > 0 )
				{
					last_connected = TRUE;
					if( updated_at != last_updated_at )
					{
						last_updated_at = updated_at;
						if( get_terminal_info() )
							TERMINAL_STATUS_INTERVAL_SECONDS = terminal.update_interval * 60;

						if( TERMINAL_STATUS_INTERVAL_SECONDS == 0 )
							TERMINAL_STATUS_INTERVAL_SECONDS = NETWORK_REQUEST_INTERVAL_SECONDS;
					}
				}
				else
					last_connected = FALSE;

				gettimeofday( &start, NULL );
				if( TERMINAL_STATUS_INTERVAL_SECONDS == 0 )
					TERMINAL_STATUS_INTERVAL_SECONDS = NETWORK_REQUEST_INTERVAL_SECONDS;
			}

			if( last_connected != terminal.connected )
			{
				terminal.connected = last_connected;
				gdk_threads_enter();
				network_event( NE_CONNECTION_STATUS_CHANGED );
				gdk_threads_leave();
			}

			if( terminal.command != CMD_NONE )
			{
				switch( terminal.command )
				{
				case CMD_REBOOT:
					if( !interaction_in_progress() )
					{
						terminal.command = CMD_NONE;
						system( "reboot" );
					}
					break;

				default:
					break;
				}
			}
		}

		nanosleep( &delay, NULL );
	}
	trace( "Leaving network thread!" );
}

gpointer update_thread_func( gpointer data )
{
    gboolean update_downloaded = FALSE;

    // trace( "Update thread started" );
	while( TRUE )
	{
		if( terminal.update_to_version > 0 && APP_VERSION != terminal.update_to_version )
		{
			if( !update_downloaded )
			{
				trace( "Update requested" );

				CURL* curl = curl_easy_init();
				if( curl )
				{
					char url[256];
					sprintf( url, "%s%d", TERMINAL_UPDATE_URL, terminal.id );

					FILE* fp = fopen( DOWNLOADS_PATH, "wb" );
					if( fp != NULL )
					{
						curl_easy_setopt( curl, CURLOPT_URL, url );
						curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, NULL );
						curl_easy_setopt( curl, CURLOPT_WRITEDATA, fp );

				        curl_easy_setopt( curl, CURLOPT_DNS_USE_GLOBAL_CACHE, 0 );
				        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );

						if( curl_easy_perform( curl ) == CURLE_OK )
						{
							if( rename( DOWNLOADS_PATH, UPDATE_PATH ) == 0 )
							{
								trace( "Update downloaded" );
								update_downloaded = TRUE;
							}
							else
								trace( "Failed to move update from downloads folder" );
						}
						else
							trace( "Failed to download update" );
						fclose( fp );
					}
					else
						trace( "Failed to create temporary file for update" );
					curl_easy_cleanup(curl);
				}
				else
					trace( "Failed to init curl" );
				if( !update_downloaded )
					sleep( 30 );
			}
			else if( !interaction_in_progress() )
			{
				trace( "Ready to update. Quitting..." );
				quit();
				break;
			}
		}
		sleep( 1 );
	}
	return NULL;
}



gboolean gprs_up()
{
    FILE* file = fopen( "/sys/class/net/ppp0/operstate", "r" );
    if( file != NULL )
    {
		char line[32];
		fgets( line, sizeof( line ), file );
		fclose( file );

		if( index_of( line, "down", 0 ) >= 0 || ( index_of( line, "unknown", 0 ) >= 0 && !terminal.connected ) )
			return FALSE;
		else
			return TRUE;
    }
    else
    {
    	file = fopen( "/sys/class/net/eth0/operstate", "r" );
    	if( file != NULL )
    	{
    		char line[32];
    		fgets( line, sizeof( line ), file );
    		fclose( file );
    		if( index_of( line, "up", 0 ) >= 0 )
    			return TRUE;
    	}
    }
    return FALSE;
}

void init_gsm_modem()
{
	trace( "init_gsm_modem()" );
	FILE* file = fopen( "/sys/class/gpio/export", "w" );
	if( file != NULL )
	{
		fprintf( file, "54" );
		fclose( file );
	}

	file = fopen( "/sys/class/gpio/gpio54/direction", "w" );
	if( file != NULL )
	{
		fprintf( file, "out" );
		fclose( file );
	}
}

gboolean gsm_modem_turned_on()
{
	int status = 1;
	FILE* file = fopen( "/sys/class/gpio/gpio7/value", "r" );
	if( file == NULL )
	{
		file = fopen( "/sys/class/gpio/export", "w" );
		if( file != NULL )
		{
			fprintf( file, "7" );
			fclose( file );

			file = fopen( "/sys/class/gpio/gpio7/direction", "w" );
			if( file != NULL )
			{
				fprintf( file, "in" );
				fclose( file );

				file = fopen( "/sys/class/gpio/gpio7/value", "r" );
			}
		}
	}
	if( file != NULL )
	{
		fscanf( file, "%d", &status );
		fclose( file );
	}
	return ( status == 0 );
}

void turn_off_gsm_modem()
{
	FILE* file = fopen( "/sys/class/gpio/gpio54/value", "w" );
	if( file != NULL )
	{
		fprintf( file, "1" );
		fclose( file );
		sleep( 3 );

		file = fopen( "/sys/class/gpio/gpio54/value", "w" );
		if( file != NULL )
		{
			fprintf( file, "0" );
			fclose( file );
		}
	}
}

gboolean turn_on_gsm_modem()
{
	trace( "turning on GPRS-modem" );
	gboolean turned_on = FALSE;

	FILE* file = fopen( "/sys/class/gpio/gpio54/value", "w" );
	if( file == NULL )
	{
		init_gsm_modem();
		file = fopen( "/sys/class/gpio/gpio54/value", "w" );
	}
	if( file != NULL )
	{
		fprintf( file, "1" );
		fclose( file );
		sleep( 1 );
		file = fopen( "/sys/class/gpio/gpio54/value", "w" );
		if( file != NULL )
		{
			fprintf( file, "0" );
			fclose( file );
		}

		for( int i  = 0; i < 7; i++ )
		{
			if( gsm_modem_turned_on() )
			{
				turned_on = TRUE;
				// trace( "GPRS-modem turned on" );
				break;
			}
			sleep( 1 );
		}

		/*
		struct termios tty;
		int fd = open( "/dev/ttymxc2", O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY );
		if( fd >= 0 )
		{
			tcgetattr( fd, &tty );

			tty.c_cflag = ( tty.c_cflag & ~CSIZE ) | CS8;
			tty.c_cflag |= B115200;
			tty.c_lflag = 0; // no signalling flag, no echo
			tty.c_iflag &= ~( IXON | IXOFF | IXANY ); // shut off xon/xoff ctrl

			if( tcsetattr( fd, TCSANOW, &tty ) != 0 )
			{
				trace( "Error initializing GPRS Modem device %d (%s) from tcsetattr", errno, strerror( errno ) );
				return FALSE;
			}

			int delay_microseconds = 250 * 1000;
			char at[] = { 'A', 'T', '\n' };

			write( fd, at, sizeof( at ) );
			usleep( delay_microseconds );

			char buf[64], c;
			int i = 0, n = 0, bufsize = sizeof( buf ), seconds_passed = 0, at_sent_at = 0;
			bzero( buf, bufsize );

		    struct timeval start, end;
		    gettimeofday( &start, NULL );

			while( TRUE )
			{
				gettimeofday( &end, NULL );
				seconds_passed = end.tv_sec - start.tv_sec;

				n = read( fd, &c, 1 );
				if( n > 0 )
				{
					if( i < bufsize )
						buf[i++] = c;
					else
					{
						trace( "GPRS buffer overflow!" );
						i = 0;
						bzero( buf, bufsize );
					}
				}
				else
				{
					if( i > 0 )
					{
						trace( "GPRS buf: %s", buf );
						if( indexOf( buf, "OK", 0 ) > -1 )
						{
							turned_on = TRUE;
							trace( "GPRS-modem turned on" );
							break;
						}
						i = 0;
						bzero( buf, bufsize );
					}
					else if( ( seconds_passed - at_sent_at ) >= 2 )
					{
						write( fd, at, sizeof( at ) );
						usleep( delay_microseconds );
						at_sent_at = seconds_passed;
						// trace( "AT" );
					}
				}

				if( seconds_passed > 7 ) // stop waiting after 7 seconds
				{
					trace( "GPRS timeout" );
					break;
				}
			}
			close( fd );
		}
		*/
	}
	return turned_on;
}

void gprs_connect()
{
	trace( "Resetting GPRS-connection..." );

	// system( "poff" );
	system( "stty 115200 -F /dev/ttymxc2 raw -echo" );
	system( "pppd call cdma" );

	trace( "pppd script finished" );
}

gpointer gprs_thread_func( gpointer data )
{
	gboolean modem_on = FALSE, modem_reset = FALSE;

	while( TRUE )
	{
		if( !gprs_up() )
		{
			modem_on = FALSE;

			if( gsm_modem_turned_on() )
			{
				if( modem_reset )
				{
					turn_off_gsm_modem();
					trace( "GSM-modem is turned on: turning off" );
					modem_reset = FALSE;
				}
				else
				{
					modem_on = TRUE;
					modem_reset = TRUE;
				}
			}
			else
			{
				trace( "GSM-modem is turned off: turning on" );
				turn_on_gsm_modem();

				if( gsm_modem_turned_on() )
				{
					modem_on = TRUE;
					trace( "GSM-modem is turned on" );
				}
				else
					trace( "GSM-modem is turned off" );
			}

			if( modem_on )
			{
				sleep( 5 );
				gprs_connect();
			}
		}
		sleep( 5 );
	}

	return NULL;
}



int main( int argc, char** argv )
{
	init();
	trace( "Onay Terminal %d\n", APP_VERSION  );

	if( argc > 1 )
	{
		for( int i = 0; i < argc; i++ )
		{
			if( strcmp( argv[i], "-t" ) == 0 )
			{
				trace_transaction();
				return 0;
			}
			else if( strcmp( argv[i], "-s" ) == 0 )
			{
				printf( "Session Amount: %d\n\n", transaction.session_amount );
				return 0;
			}
		}
	}

	curl_global_init( CURL_GLOBAL_DEFAULT );

    gdk_threads_init();
    gdk_threads_enter();

    gst_init( &argc, &argv );
	gtk_init( &argc, &argv );

	init_video();

	create_ui();
	display_form( SCREEN_INTRO );
	// display_form( SCREEN_OUT_OF_SERVICE );
	// display_form( SCREEN_HOME );
	// display_form( SCREEN_LOADING );
	// display_form( SCREEN_BALANCE_INTRO );
	// display_form( SCREEN_BALANCE_REFILL );
	// display_form( SCREEN_BALANCE_REFILLED );
	// display_form( SCREEN_BALANCE_REFILLED_WHEN_CONNECTED );
	// display_form( SCREEN_CARD_NOT_FOUND );
	// display_form( SCREEN_CARD_BOUGHT );
	// display_form( SCREEN_OUT_OF_SERVICE );
	// display_more_time_form( 30 );
	// display_form( SCREEN_NO_CARDS );
	// display_form( SCREEN_PASS_INTRO );
	// display_form( SCREEN_PASS_FIRST );
	// display_form( SCREEN_PASS_ALREADY_ACTIVE );
	// display_form( SCREEN_PASS_PARTIAL_AMOUNT );
	// display_form( SCREEN_PASS_SUFFICIENT_AMOUNT );
	// display_form( SCREEN_PASS_INSUFFICIENT_AMOUNT );
	// display_form( SCREEN_PASS_ACTIVE );
	// display_form( SCREEN_PASS_ACTIVE_NEXT_MONTH );
	// display_form( SCREEN_PASS_ACTIVE_WHEN_CONNECTED );
	// display_form( SCREEN_CARD_INTRO );
	// display_form( SCREEN_CARD_FREE );
	// display_form( SCREEN_CARD_SCAN );
	// display_form( SCREEN_MANUAL_INPUT );
	// display_form( SCREEN_PIN_CODE );

    GThread* bill_acceptor_thread = g_thread_new( NULL, bill_acceptor_thread_func, NULL );
    GThread* card_dispenser_thread = g_thread_new( NULL, card_dispenser_thread_func, NULL );
    GThread* card_reader_thread = g_thread_new( NULL, card_reader_thread_func, NULL );
    GThread* cheque_printer_thread = g_thread_new( NULL, cheque_printer_thread_func, NULL );
    GThread* network_thread = g_thread_new( NULL, network_thread_func, NULL );
    GThread* spi_thread = g_thread_new( NULL, spi_thread_func, NULL );
    GThread* update_thread = g_thread_new( NULL, update_thread_func, NULL );
    GThread* gprs_thread = g_thread_new( NULL, gprs_thread_func, NULL );

	gtk_main();

	g_thread_unref( bill_acceptor_thread );
	g_thread_unref( card_dispenser_thread );
	g_thread_unref( card_reader_thread );
	g_thread_unref( cheque_printer_thread );
	g_thread_unref( network_thread );
	g_thread_unref( spi_thread );
	g_thread_unref( update_thread );
	g_thread_unref( gprs_thread );

	free_video();

    gdk_threads_leave();
    curl_global_cleanup();
    cleanup();

	return 0;
}

