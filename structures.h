#include <stdint.h>
#define BUF_MAX_SIZE 256

typedef struct station_info_request_t {
	uint8_t type;// = 1;
} station_info_request;

//Init Function for station_info_request_t
station_info_request initStationInfoRequest(station_info_request *sir){
	sir->type = 1;
	return *sir;
}

typedef struct station_info_t {
	uint8_t type;// = 13;
	uint8_t station_number;
	uint8_t station_name_size;
	char station_name[BUF_MAX_SIZE];
	uint32_t multicast_address;
	uint16_t data_port;
	uint16_t info_port;
	uint32_t bit_rate;
} station_info;

station_info initStationInfo(station_info* si){
	si->type = 13;
	return *si;
}

typedef struct site_info_t { 
	uint8_t type;// = 10;
	uint8_t site_name_size;
	char site_name[BUF_MAX_SIZE];
	uint8_t site_desc_size;
	char site_desc[BUF_MAX_SIZE];
	uint8_t station_count;
	station_info station_list[BUF_MAX_SIZE];
} site_info;

site_info initSiteInfo(site_info *si){
	si->type = 10;	
	return *si;
}

typedef struct station_not_found_t{
	uint8_t type;// = 11;
	uint8_t station_number;
} station_not_found;

station_not_found initStationNotFound(station_not_found *snf){
	snf->type = 11;
	return *snf;
}

typedef struct song_info_t {
	uint8_t type;// = 12;
	uint8_t song_name_size;
	char song_name[BUF_MAX_SIZE];
	uint16_t remaining_time_in_sec;
	uint8_t next_song_name_size;
	char next_song_name[BUF_MAX_SIZE];
} song_info;

song_info iniSongInfo(song_info *si){
	si->type = 12;
	return *si;
}