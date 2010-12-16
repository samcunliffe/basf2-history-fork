//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Display.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger information.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#define TRGCDCDisplay_INLINE_DEFINE_HERE
#include "trg/cdc/Display.h"

namespace Belle2 {

Gtk::Main * GtkMain = 0;
bool TRGCDCDisplay::_skipEvent = false;
bool TRGCDCDisplay::_endOfEvent = false;
bool TRGCDCDisplay::_endOfEventFlag = false;

TRGCDCDisplay::TRGCDCDisplay(const std::string & name,
				     int sizeWindow,
				     int sizeMax)
    : _axial(true),
      _stereo(false),
      _wireName(false),
      _oldCDC(false),
      _box0(false, 2),
      _menuButtons(true, 2),
      _buttonNext("Next Step"),
      _buttonEndOfEvent("End of Event"),
      _buttonNextEvent("Next Eevnt"),
      _label("Stage : TRGing not started \nInformation : An event just occured",
	     Gtk::ALIGN_LEFT,
	     Gtk::ALIGN_TOP),
      _adjustment(double(sizeWindow) / sizeMax / 2,
		  double(sizeWindow) / sizeMax / 2,
		  10.0,
		  0.1),
      _scaler(_adjustment),
      _buttonPositionReset("Reset position"),
      _buttonAxial("Axial"),
      _buttonStereo("Stereo"),
      _buttonWireName("Wire Name"),
      _buttonBelleCDC("Belle CDC"),
      _w(0) {
    set_title(name);
}

TRGCDCDisplay::~TRGCDCDisplay() {
}

void
TRGCDCDisplay::initialize(TRGCDCDisplayDrawingArea & w,
			      int sizeWindow) {
    _w = & w;

    _buttonAxial.set_active();
    _axial = _buttonAxial.get_active();
    _buttonStereo.set_active(false);
    _buttonBelleCDC.set_active(false);
    _stereo = _buttonStereo.get_active();
    _buttonWireName.set_active(false);
    _wireName = _buttonWireName.get_active();

    _w->set_size_request(sizeWindow, sizeWindow);

    _buttonNext
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_next));
    _menuButtons.pack_start(_buttonNext, Gtk::PACK_EXPAND_WIDGET, 2);
    _buttonEndOfEvent
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_endOfEvent));
    _menuButtons.pack_start(_buttonEndOfEvent, Gtk::PACK_EXPAND_WIDGET, 2);
    _buttonNextEvent
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_nextEvent));
    _menuButtons.pack_start(_buttonNextEvent, Gtk::PACK_EXPAND_WIDGET, 2);

    _scaler.set_update_policy(Gtk::UPDATE_CONTINUOUS);
    _scaler.set_digits(3);
    _scaler.set_value_pos(Gtk::POS_LEFT);
    _scaler.set_draw_value();
    _scaler.set_size_request(200, 30);
    _scaler
	.signal_value_changed()
	.connect(sigc::mem_fun(* this,
			       & TRGCDCDisplay::on_scale_value_changed));
    _buttonPositionReset
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_positionReset));
    _buttonAxial
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_axial));
    _buttonStereo
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_stereo));
    _buttonWireName
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_wireName));
    _buttonBelleCDC
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_BelleCDC));
    _scale.pack_start(_scaler, Gtk::PACK_SHRINK, 5);
    _scale.pack_start(_buttonPositionReset, Gtk::PACK_EXPAND_WIDGET, 2);
    _scale.pack_start(_buttonAxial, Gtk::PACK_SHRINK, 2);
    _scale.pack_start(_buttonStereo, Gtk::PACK_SHRINK, 2);
    _scale.pack_start(_buttonWireName, Gtk::PACK_SHRINK, 2);
    _scale.pack_start(_buttonBelleCDC, Gtk::PACK_SHRINK, 2);

    _box0.pack_start(_menuButtons, Gtk::PACK_SHRINK, 5);
    _box0.pack_start(_label, Gtk::PACK_EXPAND_WIDGET, 5);
    _box0.pack_start(* _w, Gtk::PACK_EXPAND_WIDGET, 5);
    _box0.pack_start(_scale, Gtk::PACK_SHRINK, 5);

    set_border_width(5);
    add(_box0);
    show_all();    
}

void
TRGCDCDisplay::on_scale_value_changed(void) {
    const double val = scale();
    _w->scale(val);
// std::cout << "scale value=" << val << std::endl;
    _w->on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplay::on_positionReset(void) {
    _w->resetPosition();
    const double val = _w->scale();
    scale(val);
    _w->on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplay::on_next(void) {
    Gtk::Main::quit();
}

void
TRGCDCDisplay::on_endOfEvent(void) {
    _endOfEventFlag = true;
    _skipEvent = true;
    Gtk::Main::quit();
}

void
TRGCDCDisplay::on_nextEvent(void) {
    _skipEvent = true;
    Gtk::Main::quit();
}

inline
void
TRGCDCDisplay::on_axial(void) {
    _axial = _buttonAxial.get_active();
    _w->axial(_axial);
    _w->on_expose_event((GdkEventExpose *) NULL);
}

inline
void
TRGCDCDisplay::on_stereo(void) {
    _stereo = _buttonStereo.get_active();
    _w->stereo(_stereo);
    _w->on_expose_event((GdkEventExpose *) NULL);
}

inline
void
TRGCDCDisplay::on_wireName(void) {
    _wireName = _buttonWireName.get_active();
    _w->wireName(_wireName);
    _w->on_expose_event((GdkEventExpose *) NULL);
}

inline
void
TRGCDCDisplay::on_BelleCDC(void) {
    _oldCDC = _buttonBelleCDC.get_active();
    _w->oldCDC(_oldCDC);
    _w->on_expose_event((GdkEventExpose *) NULL);
}

// void
// TRGCDCDisplay::initializeGTK(void) {
//     std::cout << "TRGCDCDisplay ... initializing GTK"
// 				      << std::endl;
//     int argc = 0;
//     char ** argv = 0;
//     Gtk::Main main_instance(argc, argv);
//     GtkMain = main_instance.instance();
// }

void
TRGCDCDisplay::run(void) {
    int argc = 0;
    char ** argv = 0;
    Gtk::Main main_instance(argc, argv);
    show();
    if (((! _skip) && (! _skipEvent)) ||
	(_endOfEventFlag && _endOfEvent))
	Gtk::Main::run();
}

void
TRGCDCDisplay::clear(void) {
    _w->clear();
    const std::string id = "unknown";
    _stage = id;
    _info = "";
}

} // namespace Belle2

#endif
