#include <gtk/gtk.h>
#include <stdlib.h>
	
	typedef enum {
	MODE_NONE,
	MODE_TX,
	MODE_RX
	} AppMode;
	
	typedef struct {
	GtkApplication *app;
	GtkWidget *window;
	GtkWidget *stack;
	
	
	GtkWidget *home_page;
	GtkWidget *tx_page;
	GtkWidget *rx_page;
	GtkWidget *preset_page;
	
	GtkWidget *preset_title;
	
	/* TX */
	GtkWidget *tx_picture;
	GtkWidget *tx_overlay_label;
	guint tx_timer_id;
	int tx_frame_count;
	
	/* RX */
	GtkWidget *rx_label;
	guint rx_timer_id;
	int rx_frame_count;
	
	/* Preset widgets */
	GtkWidget *cmb_freq;
	GtkWidget *cmb_mode;
	GtkWidget *cmb_sr;
	GtkWidget *cmb_rolloff;
	
	/* Saved preset values */
	gchar *freq_mhz;
	gchar *modulation;
	gchar *symbol_rate;
	gchar *rolloff;
	
	AppMode current_mode;
	
	
	} AppData;
	
	/* =========================================================
	
	* Utility
	* ========================================================= */
	
	static void set_default_presets(AppData *ad)
	{
	g_free(ad->freq_mhz);
	g_free(ad->modulation);
	g_free(ad->symbol_rate);
	g_free(ad->rolloff);
	
	
	ad->freq_mhz    = g_strdup("438");
	ad->modulation  = g_strdup("QPSK1/4");
	ad->symbol_rate = g_strdup("333000");
	ad->rolloff     = g_strdup("0.20");
	
	
	}
	
	static void stop_timers(AppData *ad)
	{
	if (ad->tx_timer_id != 0) {
	g_source_remove(ad->tx_timer_id);
	ad->tx_timer_id = 0;
	}
	if (ad->rx_timer_id != 0) {
	g_source_remove(ad->rx_timer_id);
	ad->rx_timer_id = 0;
	}
	}
	
	static void go_home(AppData *ad)
	{
	stop_timers(ad);
	ad->current_mode = MODE_NONE;
	gtk_stack_set_visible_child(GTK_STACK(ad->stack), ad->home_page);
	}
	
	static void sync_preset_widgets_from_saved(AppData *ad)
	{
	if (g_strcmp0(ad->freq_mhz, "437") == 0) {
	gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_freq), 0);
	} else if (g_strcmp0(ad->freq_mhz, "438") == 0) {
	gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_freq), 1);
	} else if (g_strcmp0(ad->freq_mhz, "2400") == 0) {
	gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_freq), 2);
	} else {
	gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_freq), 1);
	}
	
	
	if (g_strcmp0(ad->modulation, "QPSK1/4") == 0) {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_mode), 0);
	} else if (g_strcmp0(ad->modulation, "QPSK1/2") == 0) {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_mode), 1);
	} else if (g_strcmp0(ad->modulation, "QPSK3/4") == 0) {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_mode), 2);
	} else if (g_strcmp0(ad->modulation, "8PSK3/5") == 0) {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_mode), 3);
	} else {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_mode), 0);
	}
	
	if (g_strcmp0(ad->symbol_rate, "125000") == 0) {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_sr), 0);
	} else if (g_strcmp0(ad->symbol_rate, "250000") == 0) {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_sr), 1);
	} else if (g_strcmp0(ad->symbol_rate, "333000") == 0) {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_sr), 2);
	} else if (g_strcmp0(ad->symbol_rate, "1000000") == 0) {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_sr), 3);
	} else {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_sr), 2);
	}
	
	if (g_strcmp0(ad->rolloff, "0.20") == 0) {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_rolloff), 0);
	} else if (g_strcmp0(ad->rolloff, "0.25") == 0) {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_rolloff), 1);
	} else if (g_strcmp0(ad->rolloff, "0.35") == 0) {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_rolloff), 2);
	} else {
	 gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_rolloff), 0);
	}
	
	
	}
	
	static void save_presets_from_widgets(AppData *ad)
	{
	gchar *freq = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ad->cmb_freq));
	gchar *mode = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ad->cmb_mode));
	gchar *sr   = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ad->cmb_sr));
	gchar *ro   = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ad->cmb_rolloff));
	
	
	if (freq != NULL) {
	 g_free(ad->freq_mhz);
	 ad->freq_mhz = g_strdup(freq);
	 g_free(freq);
	}
	
	if (mode != NULL) {
	 g_free(ad->modulation);
	 ad->modulation = g_strdup(mode);
	 g_free(mode);
	}
	
	if (sr != NULL) {
	 g_free(ad->symbol_rate);
	 ad->symbol_rate = g_strdup(sr);
	 g_free(sr);
	}
	
	if (ro != NULL) {
	 g_free(ad->rolloff);
	 ad->rolloff = g_strdup(ro);
	 g_free(ro);
	}
	
	g_print("Saved preset: freq=%sMHz mode=%s sr=%s rolloff=%s\n",
	     ad->freq_mhz, ad->modulation, ad->symbol_rate, ad->rolloff);
	
	
	}
	
	/* =========================================================
	
	* Command builders
	* ========================================================= */
	
	static void launch_tx_command(AppData *ad)
	{
	 long long tx_hz = g_ascii_strtoll(ad->freq_mhz, NULL, 10) * 1000000LL;
	 const char *ts_file = "test_1280x720_30fps_800k.ts";
	
	 if (g_strcmp0(ad->modulation, "QPSK1/4") == 0) {
	     ts_file = "out_800x480_av_mp2.ts";
	 }
	
	 gchar *cmd = g_strdup_printf(
	     "./start_tx.sh %lld %s %s %s",
	     tx_hz,
	     ad->modulation,
	     ad->symbol_rate,
	     ts_file
	 );
	
	 g_print("TX command: %s\n", cmd);
	 g_spawn_command_line_async(cmd, NULL);
	 g_free(cmd);
	}
	
	
	static void launch_rx_command(AppData *ad)
	{
	 long long rx_hz = g_ascii_strtoll(ad->freq_mhz, NULL, 10) * 1000000LL + 22000LL;
	
	 gchar *cmd = g_strdup_printf(
	     "./start_rx.sh %lld %s %s",
	     rx_hz,
	     ad->modulation,
	     ad->symbol_rate
	 );
	
	 g_print("RX command: %s\n", cmd);
	 g_spawn_command_line_async(cmd, NULL);
	 g_free(cmd);
	}
	
	
	static void stop_tx(AppData *ad)
	{
	 (void)ad;
	 g_spawn_command_line_async("./stop_tx.sh", NULL);
	}
	
	static void stop_rx(AppData *ad)
	{
	(void)ad;
	g_print("STOP_RX\n");
	
	
	g_spawn_command_line_async("pkill -f RF_UDP_dvbs2_rx_headless.py", NULL);
	
	}
	
	/* =========================================================
	
	* TX monitor
	* ========================================================= */
	
	static gboolean tx_monitor_tick(gpointer user_data)
	{
	AppData *ad = (AppData *)user_data;
	const char *jpg_path = "/tmp/jpg/latest.jpg";
	GError *error = NULL;
	GdkTexture *texture = NULL;
	char overlay_text[256];
	
	
	ad->tx_frame_count++;
	
	if (g_file_test(jpg_path, G_FILE_TEST_EXISTS)) {
	 texture = gdk_texture_new_from_filename(jpg_path, &error);
	 if (texture != NULL) {
	     gtk_picture_set_paintable(GTK_PICTURE(ad->tx_picture),
	                               GDK_PAINTABLE(texture));
	     g_object_unref(texture);
	
	     g_snprintf(overlay_text, sizeof(overlay_text),
	                "TX Monitor 1 fps  frame=%d\n%s MHz / %s / %s / RO %s\n(single click = stop)",
	                ad->tx_frame_count,
	                ad->freq_mhz, ad->modulation, ad->symbol_rate, ad->rolloff);
	     gtk_label_set_text(GTK_LABEL(ad->tx_overlay_label), overlay_text);
	 } else {
	     g_snprintf(overlay_text, sizeof(overlay_text),
	                "TX Monitor\nJPEG load error:\n%s",
	                error ? error->message : "unknown error");
	     gtk_label_set_text(GTK_LABEL(ad->tx_overlay_label), overlay_text);
	     if (error) {
	         g_error_free(error);
	     }
	 }
	} else {
	 g_snprintf(overlay_text, sizeof(overlay_text),
	            "TX Monitor\nwaiting for %s\n%s MHz / %s / %s / RO %s\n(single click = stop)",
	            jpg_path,
	            ad->freq_mhz, ad->modulation, ad->symbol_rate, ad->rolloff);
	 gtk_label_set_text(GTK_LABEL(ad->tx_overlay_label), overlay_text);
	}
	
	return G_SOURCE_CONTINUE;
	
	
	}
	
	/* =========================================================
	
	* RX monitor
	* ========================================================= */
	
	static gboolean rx_monitor_tick(gpointer user_data)
	{
	AppData *ad = (AppData *)user_data;
	char buf[256];
	
	
	ad->rx_frame_count++;
	g_snprintf(buf, sizeof(buf),
	        "RX Video Area\nframe=%d\n%s MHz / %s / %s / RO %s\n(single click = stop)",
	        ad->rx_frame_count,
	        ad->freq_mhz, ad->modulation, ad->symbol_rate, ad->rolloff);
	
	gtk_label_set_text(GTK_LABEL(ad->rx_label), buf);
	return G_SOURCE_CONTINUE;
	
	
	}
	
	/* =========================================================
	
	* Handlers
	* ========================================================= */
	
	static void on_tx_clicked(GtkButton *button, gpointer user_data)
	{
	 AppData *ad = (AppData *)user_data;
	 (void)button;
	
	 stop_timers(ad);
	 ad->current_mode = MODE_TX;
	 ad->tx_frame_count = 0;
	
	 launch_tx_command(ad);
	
	 gtk_label_set_text(GTK_LABEL(ad->tx_overlay_label), "TX Monitor\nstarting...");
	 gtk_stack_set_visible_child(GTK_STACK(ad->stack), ad->tx_page);
	
	 ad->tx_timer_id = g_timeout_add(1000, tx_monitor_tick, ad);
	}
	
	static void on_rx_screen_clicked(GtkGestureClick *gesture,
	                              gint n_press,
	                              gdouble x,
	                              gdouble y,
	                              gpointer user_data)
	{
	 AppData *ad = (AppData *)user_data;
	
	 if (n_press == 1) {
	     g_spawn_command_line_async("./stop_rx.sh", NULL);    	
	     go_home(ad);
	 }
	}
	
	
	static void on_rx_clicked(GtkButton *button, gpointer user_data)
	{
	 AppData *ad = (AppData *)user_data;
	 (void)button;
	
	 launch_rx_command(ad);
	
	 gtk_label_set_text(GTK_LABEL(ad->rx_label),
	                    "Receiving...\nClose ffplay window to stop");
	
	 gtk_stack_set_visible_child(GTK_STACK(ad->stack), ad->rx_page);
	}
	
	static void on_preset_done_clicked(GtkButton *button, gpointer user_data)
	{
	AppData *ad = (AppData *)user_data;
	(void)button;
	
	
	save_presets_from_widgets(ad);
	go_home(ad);
	
	
	}
	
	static void on_open_preset_clicked(GtkButton *button, gpointer user_data)
	{
	AppData *ad = (AppData *)user_data;
	(void)button;
	
	
	sync_preset_widgets_from_saved(ad);
	gtk_label_set_text(GTK_LABEL(ad->preset_title), "Preset");
	gtk_stack_set_visible_child(GTK_STACK(ad->stack), ad->preset_page);
	
	
	}
	
	static void on_tx_monitor_pressed(GtkGestureClick *gesture,
	gint n_press,
	gdouble x,
	gdouble y,
	gpointer user_data)
	{
	AppData *ad = (AppData *)user_data;
	(void)gesture;
	(void)x;
	(void)y;
	
	
	if (n_press == 1) {
	 stop_tx(ad);
	 go_home(ad);
	}
	
	
	}
	
	
	/* =========================================================
	
	* Page creation
	* ========================================================= */
	
	static GtkWidget *create_home_page(AppData *ad)
	{
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
	GtkWidget *btn_tx = gtk_button_new_with_label("TX");
	GtkWidget *btn_rx = gtk_button_new_with_label("RX");
	GtkWidget *btn_preset = gtk_button_new_with_label("Preset");
	
	
	gtk_widget_set_margin_top(box, 20);
	gtk_widget_set_margin_bottom(box, 20);
	gtk_widget_set_margin_start(box, 20);
	gtk_widget_set_margin_end(box, 20);
	
	gtk_widget_set_hexpand(btn_tx, TRUE);
	gtk_widget_set_vexpand(btn_tx, TRUE);
	
	gtk_widget_set_hexpand(btn_rx, TRUE);
	gtk_widget_set_vexpand(btn_rx, TRUE);
	
	gtk_widget_set_hexpand(btn_preset, TRUE);
	gtk_widget_set_vexpand(btn_preset, TRUE);
	
	gtk_box_append(GTK_BOX(box), btn_tx);
	gtk_box_append(GTK_BOX(box), btn_rx);
	gtk_box_append(GTK_BOX(box), btn_preset);
	
	g_signal_connect(btn_tx, "clicked", G_CALLBACK(on_tx_clicked), ad);
	g_signal_connect(btn_rx, "clicked", G_CALLBACK(on_rx_clicked), ad);
	g_signal_connect(btn_preset, "clicked", G_CALLBACK(on_open_preset_clicked), ad);
	
	return box;
	
	
	}
	
	static GtkWidget *create_tx_page(AppData *ad)
	{
	GtkWidget *overlay = gtk_overlay_new();
	GtkWidget *frame = gtk_frame_new(NULL);
	GtkGesture *gesture;
	
	
	ad->tx_picture = gtk_picture_new();
	gtk_picture_set_can_shrink(GTK_PICTURE(ad->tx_picture), TRUE);
	gtk_widget_set_hexpand(ad->tx_picture, TRUE);
	gtk_widget_set_vexpand(ad->tx_picture, TRUE);
	
	ad->tx_overlay_label = gtk_label_new("TX Monitor\n1 fps");
	gtk_widget_set_halign(ad->tx_overlay_label, GTK_ALIGN_START);
	gtk_widget_set_valign(ad->tx_overlay_label, GTK_ALIGN_START);
	gtk_widget_set_margin_top(ad->tx_overlay_label, 12);
	gtk_widget_set_margin_start(ad->tx_overlay_label, 12);
	
	gtk_frame_set_child(GTK_FRAME(frame), ad->tx_picture);
	gtk_overlay_set_child(GTK_OVERLAY(overlay), frame);
	gtk_overlay_add_overlay(GTK_OVERLAY(overlay), ad->tx_overlay_label);
	
	gesture = gtk_gesture_click_new();
	gtk_widget_add_controller(overlay, GTK_EVENT_CONTROLLER(gesture));
	g_signal_connect(gesture, "pressed", G_CALLBACK(on_tx_monitor_pressed), ad);
	
	return overlay;
	
	
	}
	
	static GtkWidget *create_rx_page(AppData *ad)
	{
	 GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	
	 ad->rx_label = gtk_label_new("Receiving...\nClose ffplay window to stop");
	 gtk_label_set_justify(GTK_LABEL(ad->rx_label), GTK_JUSTIFY_CENTER);
	
	 gtk_widget_set_hexpand(ad->rx_label, TRUE);
	 gtk_widget_set_vexpand(ad->rx_label, TRUE);
	
	 gtk_box_append(GTK_BOX(box), ad->rx_label);
	 GtkGesture *gesture = gtk_gesture_click_new();
	 gtk_widget_add_controller(box, GTK_EVENT_CONTROLLER(gesture));
	 g_signal_connect(gesture, "pressed", G_CALLBACK(on_rx_screen_clicked), ad);
	
	 return box;
	}
	
	static GtkWidget *create_preset_page(AppData *ad)
	{
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	GtkWidget *grid = gtk_grid_new();
	GtkWidget *done_btn = gtk_button_new_with_label("プリセット終了");
	
	
	GtkWidget *lbl0 = gtk_label_new("Frequency (MHz)");
	GtkWidget *lbl1 = gtk_label_new("Mode");
	GtkWidget *lbl2 = gtk_label_new("Symbol Rate");
	GtkWidget *lbl3 = gtk_label_new("Roll-off");
	
	ad->cmb_freq = gtk_combo_box_text_new();
	ad->cmb_mode = gtk_combo_box_text_new();
	ad->cmb_sr = gtk_combo_box_text_new();
	ad->cmb_rolloff = gtk_combo_box_text_new();
	
	ad->preset_title = gtk_label_new("Preset");
	
	gtk_widget_set_margin_top(box, 20);
	gtk_widget_set_margin_bottom(box, 20);
	gtk_widget_set_margin_start(box, 20);
	gtk_widget_set_margin_end(box, 20);
	
	gtk_label_set_xalign(GTK_LABEL(ad->preset_title), 0.0f);
	gtk_box_append(GTK_BOX(box), ad->preset_title);
	
	gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
	
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_freq), "437");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_freq), "438");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_freq), "2400");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_freq), 1);
	
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_mode), "QPSK1/4");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_mode), "QPSK1/2");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_mode), "QPSK3/4");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_mode), "8PSK3/5");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_mode), 0);
	
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_sr), "125000");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_sr), "250000");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_sr), "333000");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_sr), "1000000");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_sr), 2);
	
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_rolloff), "0.20");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_rolloff), "0.25");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ad->cmb_rolloff), "0.35");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ad->cmb_rolloff), 0);
	
	gtk_grid_attach(GTK_GRID(grid), lbl0, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), ad->cmb_freq, 1, 0, 1, 1);
	
	gtk_grid_attach(GTK_GRID(grid), lbl1, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), ad->cmb_mode, 1, 1, 1, 1);
	
	gtk_grid_attach(GTK_GRID(grid), lbl2, 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), ad->cmb_sr, 1, 2, 1, 1);
	
	gtk_grid_attach(GTK_GRID(grid), lbl3, 0, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), ad->cmb_rolloff, 1, 3, 1, 1);
	
	gtk_box_append(GTK_BOX(box), grid);
	
	gtk_widget_set_margin_top(done_btn, 20);
	gtk_box_append(GTK_BOX(box), done_btn);
	
	g_signal_connect(done_btn, "clicked", G_CALLBACK(on_preset_done_clicked), ad);
	
	return box;
	
	
	}
	
	/* =========================================================
	
	* Activate / main
	* ========================================================= */
	
	static void activate(GtkApplication *app, gpointer user_data)
	{
	AppData *ad = g_new0(AppData, 1);
	ad->app = app;
	(void)user_data;
	
	
	set_default_presets(ad);
	
	ad->window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(ad->window), "DVB-S2 KISS UI");
	gtk_window_set_default_size(GTK_WINDOW(ad->window), 800, 480);
	
	ad->stack = gtk_stack_new();
	gtk_window_set_child(GTK_WINDOW(ad->window), ad->stack);
	
	ad->home_page   = create_home_page(ad);
	ad->tx_page     = create_tx_page(ad);
	ad->rx_page     = create_rx_page(ad);
	ad->preset_page = create_preset_page(ad);
	
	gtk_stack_add_named(GTK_STACK(ad->stack), ad->home_page, "home");
	gtk_stack_add_named(GTK_STACK(ad->stack), ad->tx_page, "tx");
	gtk_stack_add_named(GTK_STACK(ad->stack), ad->rx_page, "rx");
	gtk_stack_add_named(GTK_STACK(ad->stack), ad->preset_page, "preset");
	
	gtk_stack_set_visible_child(GTK_STACK(ad->stack), ad->home_page);
	
	gtk_window_present(GTK_WINDOW(ad->window));
	
	
	}
	
	int main(int argc, char **argv)
	{
	GtkApplication *app;
	int status;
	
	
	app = gtk_application_new("com.example.dvbs2kiss", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	
	status = g_application_run(G_APPLICATION(app), argc, argv);
	
	g_object_unref(app);
	return status;
	
	
	}