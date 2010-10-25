//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerDisplayDrawingArea.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger objects.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef CDCTRIGGER_DISPLAY

#define CDCTRIGGER_SHORT_NAMES

#include <iostream>
#include <pangomm/init.h>
#include "trigger/cdc/CDCTriggerWire.h"
#include "trigger/cdc/CDCTriggerWireHit.h"
#include "trigger/cdc/CDCTriggerTrackSegment.h"
#include "trigger/cdc/CDCTriggerDisplayDrawingArea.h"

using namespace std;

namespace Belle2 {

CDCTriggerDisplayDrawingArea::CDCTriggerDisplayDrawingArea(int size,
							   double innerR,
							   double outerR)
    : _scale(double(size) / outerR / 2),
      _axial(true),
      _stereo(false),
      _wireName(false),
      _oldCDC(false),
      _x(0),
      _y(0),
      _innerR(innerR),
      _outerR(outerR) {
    _blue = Gdk::Color("blue");
    _red = Gdk::Color("red");
    _green = Gdk::Color("green");
    _black = Gdk::Color("black");
    _white = Gdk::Color("white");
    _grey = Gdk::Color("grey");
    _yellow = Gdk::Color("yellow");
    _grey0 = Gdk::Color("gray90");

    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(_blue);
    colormap->alloc_color(_red);
    colormap->alloc_color(_green);
    colormap->alloc_color(_black);
    colormap->alloc_color(_white);
    colormap->alloc_color(_grey);
    colormap->alloc_color(_yellow);

    Pango::init();
    Glib::ustring wn = "unknown";
    _pl = create_pango_layout(wn);

    add_events(Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK);

    cout << "Drawing called ..." << endl;
}

CDCTriggerDisplayDrawingArea::~CDCTriggerDisplayDrawingArea() {
}

void
CDCTriggerDisplayDrawingArea::on_realize() {
    Gtk::DrawingArea::on_realize();
    _window = get_window();
    _gc = Gdk::GC::create(_window);
    _window->set_background(_white);
    _window->clear();
}

bool
CDCTriggerDisplayDrawingArea::on_expose_event(GdkEventExpose *) {
    Glib::RefPtr<Gdk::Window> window = get_window();
    window->get_geometry(_winx, _winy, _winw, _winh, _wind);
    window->clear();

// std::cout << "_x,_y,_scale=" << _x << "," << _y << "," << _scale
// 	      << std::endl;

    drawCDC();
    draw();
    return true;
}

bool
CDCTriggerDisplayDrawingArea::on_button_press_event(GdkEventButton * e) {
    _x = xR(e->x);
    _y = yR(- e->y);
    on_expose_event((GdkEventExpose *) NULL);
    return true;
}

void
CDCTriggerDisplayDrawingArea::drawCDC(void) {

    //...Axis...
    _gc->set_foreground(_grey);
    _gc->set_line_attributes(1,
			     Gdk::LINE_SOLID,
			     Gdk::CAP_NOT_LAST,
			     Gdk::JOIN_MITER);
    _window->draw_line(_gc,
		       x(- _outerR),
		       y(0),
		       x(_outerR),
		       y(0));
    _window->draw_line(_gc,
		       x(0),
		       y(- _outerR),
		       x(0),
		       y(_outerR));

    //...CDC...
    _gc->set_foreground(_grey);
    _gc->set_line_attributes(1,
			     Gdk::LINE_SOLID,
			     Gdk::CAP_NOT_LAST,
			     Gdk::JOIN_MITER);
    _window->draw_arc(_gc,
		      0,
		      x(- _innerR),
		      y(_innerR),
 		      int(2 * _innerR * _scale),
 		      int(2 * _innerR * _scale),
		      0,
		      360 * 64);
    _window->draw_arc(_gc,
		      0,
		      x(- _outerR),
		      y(_outerR),
 		      int(2 * _outerR * _scale),
 		      int(2 * _outerR * _scale),
		      0,
		      360 * 64);

    //...Belle CDC...
    if (_oldCDC) {
	const double innerR = 118;
	const double outerR = 880;
	_gc->set_foreground(_grey);
	_gc->set_line_attributes(1,
				 Gdk::LINE_ON_OFF_DASH,
				 Gdk::CAP_NOT_LAST,
				 Gdk::JOIN_MITER);
	_window->draw_arc(_gc,
			  0,
			  x(- innerR),
			  y(innerR),
			  int(2 * innerR * _scale),
			  int(2 * innerR * _scale),
			  0,
			  360 * 64);
	_window->draw_arc(_gc,
			  0,
			  x(- outerR),
			  y(outerR),
			  int(2 * outerR * _scale),
			  int(2 * outerR * _scale),
			  0,
			  360 * 64);
    }
}

void
CDCTriggerDisplayDrawingArea::draw(void) {
    drawHits();
    for (unsigned i = 0; i < _segments.size(); i++)
	drawTrackSegment(* _segments[i],
			 1,
			 _segmentsColor[i],
			 Gdk::LINE_SOLID);

//     unsigned n = _objects.length();
//     for (unsigned i = 0; i < n; i++) {
// 	const TTrackBase & track = * _objects[i];
// 	if (track.objectType() == TrackBase)
// 	    drawBase(track, * _colors[i]);
// // 	else if (track.objectType() == Line)
// // 	    drawLine((const TLine &) track, * _colors[i]);
//  	else if (track.objectType() == Track)
//  	    drawTrack((const TTrack &) track, * _colors[i]);
//  	else if (track.objectType() == Segment)
//  	    drawSegment((const TSegment &) track, * _colors[i]);
//  	else if (track.objectType() == Circle)
//  	    drawCircle((const TCircle &) track, * _colors[i]);
// 	else
// 	    std::cout << "CDCTriggerDisplayDrawingArea::draw !!! can't display"
// 		      << std::endl;
//     }
}

void
CDCTriggerDisplayDrawingArea::drawHits(void) {
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    const unsigned n = _hits.size();
    for (unsigned i = 0; i < n; i++) {
 	if (! _stereo)
 	    if (_hits[i]->wire().stereo())
 		continue;
 	if (! _axial)
 	    if (_hits[i]->wire().axial())
 		continue;

	//...Points...
	const CTWire & w = _hits[i]->wire();
	const HepGeom::Point3D<double> & p = w.forwardPosition();
	double radius = _hits[i]->drift();

	colormap->alloc_color(_hitsColor[i]);
	_gc->set_foreground(_hitsColor[i]);
	_gc->set_line_attributes(1,
				 Gdk::LINE_SOLID,
				 Gdk::CAP_NOT_LAST,
				 Gdk::JOIN_MITER);
	_window->draw_arc(_gc,
			  0,
			  x((p.x() - radius) * 10),
			  y((p.y() + radius) * 10),
			  int(2 * radius * 10 * _scale),
			  int(2 * radius * 10 * _scale),
			  0,
			  360 * 64);
 	if (_wireName) {
	    Glib::ustring wn = _hits[i]->wire().name().c_str();
	    _pl->set_text(wn);
	    _window->draw_layout(_gc, x(p.x() * 10.), y(p.y() * 10.), _pl);
	}
	colormap->free_color(_hitsColor[i]);
    }
}

void
CDCTriggerDisplayDrawingArea::drawWire(const CTWire & w,
				       int lineWidth,
				       Gdk::Color & c,
				       Gdk::LineStyle s) {

    if (! _stereo)
	if (w.stereo())
	    return;
    if (! _axial)
	if (w.axial())
	    return;

    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(c);
    _gc->set_foreground(c);
    _gc->set_line_attributes(lineWidth,
			     s,
			     Gdk::CAP_NOT_LAST,
			     Gdk::JOIN_MITER);

    //...Cell shape...
    Gdk::Point p0;
    std::vector<Gdk::Point> points;
    const unsigned nDivisions = 5;
    const float ri = w.layer().innerRadius();
    const float ro = w.layer().outerRadius();
    const float cPhi = w.forwardPosition().phi();
    const float dPhi = M_PI / w.layer().nWires();
    for (unsigned j = 0; j < nDivisions + 1; j++) {  // inner
	const float phi = cPhi - dPhi
	    + (2 * dPhi / float(nDivisions)) * float(j);
	const float xx = ri * cos(phi) * 10;
	const float yy = ri * sin(phi) * 10;
	if (j == 0) {
	    p0.set_x(x(xx));
	    p0.set_y(y(yy));
	}
	points.push_back(Gdk::Point(x(xx), y(yy)));
    }
    for (unsigned j = 0; j < nDivisions + 1; j++) {  // outer
	const float phi = cPhi + dPhi
	    - (2 * dPhi / float(nDivisions)) * float(j);
	const float xx = ro * cos(phi) * 10;
	const float yy = ro * sin(phi) * 10;
	points.push_back(Gdk::Point(x(xx), y(yy)));
    }
    points.push_back(p0);

    _gc->set_foreground(c);
    _window->draw_lines(_gc, points);
}

void
CDCTriggerDisplayDrawingArea::drawTrackSegment(const CTTSegment & w,
					       int lineWidth,
					       Gdk::Color & c,
					       Gdk::LineStyle s) {
    const std::vector<const CTWire *> wires = w.wires();
    const unsigned n = wires.size();
    for (unsigned i = 0; i < n; i++) {
	drawWire(* wires[i], lineWidth, c, s);
    }
}

// void
// CDCTriggerDisplayDrawingArea::drawTrack(const TTrack & t, Gdk::Color & c) {
//     Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
//     colormap->alloc_color(c);
//     _gc->set_foreground(c);
//     _gc->set_line_attributes(1,
// 			     Gdk::LINE_SOLID,
// 			     Gdk::CAP_NOT_LAST,
// 			     Gdk::JOIN_MITER);

//     const AList<TLink> & links = t.links();
//     unsigned n = links.length();
//     for (unsigned i = 0; i < n; i++) {
// 	if (links[i]->wire() == NULL) continue;
//  	if (! _stereo)
//  	    if (links[i]->wire()->stereo())
//  		continue;
//  	if (! _axial)
//  	    if (links[i]->wire()->axial())
//  		continue;

// 	//...Points...
// 	//	const HepGeom::Point3D<double> & p = links[i]->wire()->forwardPosition();
// 	const HepGeom::Point3D<double> & p = links[i]->positionOnWire();
// 	double radius = links[i]->hit()->drift();
// 	_window->draw_arc(_gc,
// 			  0,
// 			  x((p.x() - radius) * 10),
// 			  y((p.y() + radius) * 10),
// 			  int(2 * radius * 10 * _scale),
// 			  int(2 * radius * 10 * _scale),
// 			  0,
// 			  360 * 64);
//  	if (_wireName) {
// 	    Glib::ustring wn = links[i]->wire()->name().c_str();
// 	    _pl->set_text(wn);
// 	    _window->draw_layout(_gc, x(p.x() * 10.), y(p.y() * 10.), _pl);
// 	}
//     }

//     //...Check a track...
//     if (t.cores().length() == 0) {
// 	t.dump("detail", "");
// 	colormap->free_color(c);
// 	return;
//     }

//     //...Draw a track...
//     THelix hIp = t.helix();
//     hIp.pivot(ORIGIN);
//     const HepGeom::Point3D<double> & h = hIp.center();
//     double radius = fabs(t.radius());
//     const HepGeom::Point3D<double> pIn = TLink::innerMost(t.cores())->positionOnTrack() - h;
//     const HepGeom::Point3D<double> pOut = TLink::outerMost(t.cores())->positionOnTrack() - h;
//     double a0 = atan2(pIn.y(), pIn.x()) / M_PI * 180;
//     double a1 = atan2(pOut.y(), pOut.x()) / M_PI * 180;
// //     std::cout << "h=" << h << ",r=" << radius
// // 				      << ",a0=" << a0 << ",a1=" << a1
// // 				      << std::endl;
//     double d = a1 - a0;
//     if (d > 180) d -= 360;
//     else if (d < -180) d += 360;
//     _window->draw_arc(_gc,
// 		      0,
// 		      x((h.x() - radius) * 10),
// 		      y((h.y() + radius) * 10),
// 		      int(2 * radius * 10 * _scale),
// 		      int(2 * radius * 10 * _scale),
// 		      int(a0 * 64),
// 		      int(d * 64));

//     //...Track name...
//     const HepGeom::Point3D<double> pn = TLink::outerMost(t.cores())->positionOnTrack();
//     Glib::ustring wn = t.name();
//     _pl->set_text(wn);
//     _window->draw_layout(_gc, x(pn.x() * 10.), y(pn.y() * 10.), _pl);

// //     std::cout << "h=" << h << ",r=" << radius
// // 				      << ",a0=" << a0 << ",a1=" << a1
// // 				      << std::endl;

//     colormap->free_color(c);
// }

void
CDCTriggerDisplayDrawingArea::resetPosition(void) {
    if (_winw < _winh)
	_scale = double(_winw) / _outerR / 2;
    else
	_scale = double(_winh) / _outerR / 2;

    _x = _y = 0;
    on_expose_event((GdkEventExpose *) NULL);
}

void
CDCTriggerDisplayDrawingArea::append(const std::vector<const CTWHit *> & l,
				     Gdk::Color c) {
    const unsigned n = l.size();
    for (unsigned i = 0; i < n; i++) {
	_hits.push_back(l[i]);
	_hitsColor.push_back(c);
    }
    on_expose_event((GdkEventExpose *) NULL);
}

void
CDCTriggerDisplayDrawingArea::append(const CTTSegment & s,
				     Gdk::Color c) {
    _segments.push_back(& s);
    _segmentsColor.push_back(c);
    on_expose_event((GdkEventExpose *) NULL);
}

void
CDCTriggerDisplayDrawingArea::append(const std::vector<const CTTSegment *> & l,
				     Gdk::Color c) {
    const unsigned n = l.size();
    for (unsigned i = 0; i < n; i++) {
	_segments.push_back(l[i]);
	_segmentsColor.push_back(c);
    }
    on_expose_event((GdkEventExpose *) NULL);
}

// void
// CDCTriggerDisplayDrawingArea::append(const AList<TLink> & list, Gdk::Color c) {
//     TTrackBase * t = new TTrackBase(list);
//     _selfObjects.append(t);
//     _objects.append(t);
//     _colors.append(new Gdk::Color(c));
//     on_expose_event((GdkEventExpose *) NULL);
// }

// void
// CDCTriggerDisplayDrawingArea::append(const AList<TSegment> & list,
// 				    Gdk::Color c) {
//     for (unsigned i = 0; i < list.length(); i++) {
// 	TSegment * s = new TSegment(* list[i]);
// 	_selfObjects.append(s);
// 	_objects.append(s);
// 	_colors.append(new Gdk::Color(c));
//     }
//     on_expose_event((GdkEventExpose *) NULL);
// }

// void
// CDCTriggerDisplayDrawingArea::append(const AList<TTrack> & list, Gdk::Color c) {
//     for (unsigned i = 0; i < list.length(); i++) {
// 	TTrack * s = new TTrack(* list[i]);
// 	_selfObjects.append(s);
// 	_objects.append(s);
// 	_colors.append(new Gdk::Color(c));
//     }
//     on_expose_event((GdkEventExpose *) NULL);
// }

// void
// CDCTriggerDisplayDrawingArea::drawCircle(const TCircle & t, Gdk::Color & c) {
//     Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
//     colormap->alloc_color(c);
//     _gc->set_foreground(c);
//     _gc->set_line_attributes(1,
// 			     Gdk::LINE_SOLID,
// 			     Gdk::CAP_NOT_LAST,
// 			     Gdk::JOIN_MITER);

//     const AList<TLink> & links = t.links();
//     unsigned n = links.length();
//     for (unsigned i = 0; i < n; i++) {
// 	if (links[i]->wire() == NULL) continue;
//  	if (! _stereo)
//  	    if (links[i]->wire()->stereo())
//  		continue;
//  	if (! _axial)
//  	    if (links[i]->wire()->axial())
//  		continue;

// 	//...Points...
// 	//	const HepGeom::Point3D<double> & p = links[i]->wire()->forwardPosition();
// 	const HepGeom::Point3D<double> & p = links[i]->positionOnWire();
// 	double radius = links[i]->hit()->drift();
// 	_window->draw_arc(_gc,
// 			  0,
// 			  x((p.x() - radius) * 10),
// 			  y((p.y() + radius) * 10),
// 			  int(2 * radius * 10 * _scale),
// 			  int(2 * radius * 10 * _scale),
// 			  0,
// 			  360 * 64);
//     }

//     //...Draw a circle...
//     const HepGeom::Point3D<double> & h = t.center();
//     double radius = fabs(t.radius());
//     _window->draw_arc(_gc,
// 		      0,
// 		      x((h.x() - radius) * 10),
// 		      y((h.y() + radius) * 10),
// 		      int(2 * radius * 10 * _scale),
// 		      int(2 * radius * 10 * _scale),
// 		      0,
// 		      360 * 64);
//     colormap->free_color(c);
// }

// void
// CDCTriggerDisplayDrawingArea::append(const AList<TCircle> & list,
// 				    Gdk::Color c) {
//     for (unsigned i = 0; i < list.length(); i++) {
// 	TCircle * s = new TCircle(* list[i]);
// 	_selfObjects.append(s);
// 	_objects.append(s);
// 	_colors.append(new Gdk::Color(c));
//     }
//     on_expose_event((GdkEventExpose *) NULL);
// }

void
CDCTriggerDisplayDrawingArea::clear(void) {
    _hits.clear();
    _hitsColor.clear();
    _segments.clear();
    _segmentsColor.clear();
}

} // namespace Belle2

#endif
