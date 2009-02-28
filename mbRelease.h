#pragma once

class mbRelease
{
public:
	mbRelease(const char *_title, const char *_id, const char *_artist, const char *_artist_id);
	~mbRelease();
	mbTrack *addTrack(const char *_title, const char *_id);
	void setDate(const char *_date);
	mbTrack *getTrack(unsigned int number);
	char *getTitle();
	char *getId();
	char *getArtist();
	char *getArtistId();
	char *getDate();
	unsigned int getTracksCount();
	bool va;

private:
	unsigned int size;
	mbTrack **tracks;
	char *artist;
	char *title;
	char *artist_id;
	char *id;
	char *date;
};
