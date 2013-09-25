package b2dqm.graphics;

import java.util.Iterator;

import b2daq.core.NumberString;
import b2daq.graphics.GCanvas;
import b2daq.graphics.GRect;
import b2daq.graphics.GText;
import b2daq.graphics.HtmlColor;
import b2dqm.core.Histo;
import b2dqm.core.Histo2;


public class GHisto2 extends GHisto {

	private GRect[][] _rect_v = null;
	private GRect[][] _rect_frame_v = null;
	private GText[][] _text_v = null;
	
	public GHisto2() {
		super();
		setLineColor(HtmlColor.NULL);
	}
	
	public GHisto2(Canvas canvas, Histo2 histo) {
		super(canvas, histo);
		_axis_y = canvas.getColorAxis();
		_rect_v = new GRect[histo.getAxisY().getNbins()][histo.getAxisX().getNbins()];
		_rect_frame_v = new GRect[histo.getAxisY().getNbins()][histo.getAxisX().getNbins()];
		_text_v = new GText[histo.getAxisY().getNbins()][histo.getAxisX().getNbins()];
		for (int ny = 0; ny < histo.getAxisY().getNbins(); ny++ ) {
			for (int nx = 0; nx < histo.getAxisX().getNbins(); nx++ ) {
				_rect_v[ny][nx] = new GRect();
				_rect_frame_v[ny][nx] = new GRect();
				_text_v[ny][nx] = new GText();
			}
		}
		setLineColor(HtmlColor.NULL);
	}
	
	public void update() {
		super.update();
		HtmlColor [] color_pattern = ((GColorAxis)_canvas.getColorAxis()).getColorPattern();
    	double x0 = _canvas.getPad().getX();
    	double y0 = _canvas.getPad().getY() + _canvas.getPad().getHeight();
    	double xmin = _histo.getAxisX().getMin(), xmax = _histo.getAxisX().getMax();
    	double dbinx = (xmax - xmin) / _histo.getAxisX().getNbins();
    	double ymin = _histo.getAxisY().getMin(), ymax = _histo.getAxisY().getMax();
    	double dbiny = (ymax - ymin) / _histo.getAxisY().getNbins();
		double max = _axis_y.get().getMax();
		double min = _axis_y.get().getMin();
		double X, Y, width, height;
		int n_digit = 1;
		if ( _histo.getDataType().matches(".*I") ||
			 _histo.getDataType().matches(".*C") || _histo.getDataType().matches(".*S")) {
			n_digit = 0;			
		}
		for ( int ny = 0; ny < _histo.getAxisY().getNbins(); ny++ ) {
			for ( int nx = 0; nx < _histo.getAxisX().getNbins(); nx++ ) {
				X = x0 + _canvas.getAxisX().eval( dbinx * nx + xmin );
				Y = y0 - _canvas.getAxisY().eval( dbiny * ny + ymin );
				width = x0 + _canvas.getAxisX().eval( dbinx * ( nx + 1 ) + xmin ) - X;
				height = Y - (y0 - _canvas.getAxisY().eval( dbiny * ( ny + 1 ) + ymin ));
				HtmlColor color = color_pattern[0];
				double data = _histo.getBinContent(nx, ny);
				_text_v[ny][nx].setText(NumberString.toString(data, n_digit));
				if ( data < max && data > min ) {
					color = color_pattern[(int)(color_pattern.length*((data - min)/(max - min)))];
				}
				if ( data >= max ) {
					color = color_pattern[color_pattern.length - 1];
				}
				if ( data == 0 && min >= 0 ) {
					color = HtmlColor.WHITE;
				}
				_rect_v[ny][nx].setFillColor(color);
				_rect_v[ny][nx].setLineColor(HtmlColor.NULL);
				_rect_frame_v[ny][nx].setLineColor(getLineColor());
				_rect_frame_v[ny][nx].setLineWidth(getLineWidth());
				_rect_v[ny][nx].set(X, Y - height*1.2, width*1.2, height*1.2);
				_rect_frame_v[ny][nx].set(X, Y - height, width, height);
				_text_v[ny][nx].setFont(getFont());
				_text_v[ny][nx].setPosition(X + width / 2, Y - height / 2);
				_text_v[ny][nx].setAligment("center middle");
			}
        }
	}
	
	public String getMessage(double x, double y) {
		_selected_channel_ref = null;
		if (!_canvas.getPad().hit(x, y)) return "";
		for ( int nx = 0; nx < _histo.getAxisX().getNbins(); nx++ ) {
			for ( int ny = 0; ny < _histo.getAxisY().getNbins(); ny++ ) {
				if ( _rect_v[ny][nx].hit(x, y) ) {
					Iterator<HyperRef> it = _channel_ref_v.iterator();
					while ( it.hasNext() ) {
						HyperRef ref = it.next();
						if ( nx == ref.getChannelX() && ny == ref.getChannelY() ) {
							_selected_channel_ref = ref;
							break;
						}
					}
        			return "<html>" +
        				"histo = " + get().getName() + "<br />" +
        				"bin = " + (nx+1) +", " + (ny+1) + " <br />"  +
        				"value = " + NumberString.toString(((Histo)get()).getBinContent(nx, ny), 2) +  
        				"</html>";
        		}
    		}
    	}
		return "";
	}

	public void draw(GCanvas canvas) {
		update();
		canvas.store();
		for ( int ny = 0; ny < _histo.getAxisY().getNbins(); ny++ ) {
			for ( int nx = 0; nx < _histo.getAxisX().getNbins(); nx++ ) {
				_rect_v[ny][nx].draw(canvas);
				if ( getDrawOption().matches(".*T.*") ) _text_v[ny][nx].draw(canvas);
			}
		}
		for ( int ny = 0; ny < _histo.getAxisY().getNbins(); ny++ ) {
			for ( int nx = 0; nx < _histo.getAxisX().getNbins(); nx++ ) {
				_rect_frame_v[ny][nx].draw(canvas);
			}
		}
		canvas.restore();
 	}

}
