package b2daq.dqm.ui.property;

import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.graphics.GObject;
import b2daq.graphics.GRect;

public class GRectProperty extends GShapeProperty {

	public final static String ELEMENT_LABEL = "rect";
	
	protected GRect _rect = new GRect();
	
	public GRectProperty(HistogramCanvas canvas) {
		super(canvas);
		canvas.addShape(_rect);
		_shape = _rect;
	}

	@Override
	public String getType() {
		return ELEMENT_LABEL;
	}

	@Override
	public void put(String key, String value) {
		super.put(key, value);
		if ( key.matches("x") ) {
			_rect.setX(Double.parseDouble(value));
		} else if ( key.matches("y") ) {
			_rect.setY(Double.parseDouble(value));
		} else if ( key.matches("width") ) {
			_rect.setWidth(Double.parseDouble(value));
		} else if ( key.matches("height") ) {
			_rect.setHeight(Double.parseDouble(value));
		}
	}
	
	@Override
	public void put(GProperty pro) {}

	@Override
	public GObject set(HistogramCanvas canvas, HistoPackage pack) {
		return super.set(canvas, pack);
	}

}
