

/* Lastfm Helper */

void lastfm_track_love_action(GtkAction *action, SoundmenuPlugin    *soundmenu);
void lastfm_track_unlove_action (GtkAction *action, SoundmenuPlugin    *soundmenu);
void lastfm_artist_info_action(GtkAction *action, SoundmenuPlugin    *soundmenu);
gboolean lastfm_love_handler (SoundmenuPlugin  *soundmenu);
gboolean lastfm_scrob_handler (SoundmenuPlugin  *soundmenu);
gboolean lastfm_now_playing_handler (gpointer data);
void update_lastfm (SoundmenuPlugin    *soundmenu);