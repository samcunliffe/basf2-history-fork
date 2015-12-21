package org.belle2.daq.graphics;

public abstract class GShape implements GObject {

    protected String name = "";
    protected FillProperty fill = new FillProperty();
    protected LineProperty line = new LineProperty();
    protected FontProperty font = new FontProperty();
    protected boolean focused = false;
    protected boolean visible = true;

    public GShape() {
    }

    public GShape(HtmlColor fill_color, HtmlColor line_color) {
        fill.setColor(fill_color);
        line.setColor(line_color);
    }

    public void setFill(FillProperty fill) {
        this.fill = fill;
    }

    public void setFillColor(HtmlColor color) {
        if (fill != null) {
            fill.setColor(color);
        }
    }

    public void setFillTransparency(double t) {
        if (fill != null) {
            fill.setTransparency(t);
        }
    }

    public FillProperty getFill() {
        return fill;
    }

    public HtmlColor getFillColor() {
        if (fill == null) {
            return null;
        } else {
            return fill.getColor();
        }
    }

    public double getFillTransparency() {
        if (fill == null) {
            return 0;
        } else {
            return fill.getTransparency();
        }
    }

    public void setLine(LineProperty line) {
        this.line = line;
    }

    public void setLineColor(HtmlColor color) {
        if (line != null) {
            line.setColor(color);
        }
    }

    public void setLineWidth(float width) {
        if (line != null) {
            line.setWidth(width);
        }
    }

    public void setLineStyle(String style) {
        if (line != null) {
            line.setStyle(style);
        }
    }

    public LineProperty getLine() {
        return line;
    }

    public HtmlColor getLineColor() {
        if (line == null) {
            return null;
        } else {
            return line.getColor();
        }
    }

    public double getLineWidth() {
        if (line == null) {
            return 0;
        } else {
            return line.getWidth();
        }
    }

    public String getLineStyle() {
        if (line == null) {
            return "";
        } else {
            return line.getStyle();
        }
    }


    public FontProperty getFont() {
        return font;
    }

    public HtmlColor getFontColor() {
        return font.getColor();
    }

    public double getFontSize() {
        return font.getSize();
    }

    public String getFontFamily() {
        return font.getFamily();
    }

    public int getWeight() {
        return font.getWeight();
    }

    public void setFont(FontProperty font) {
        this.font = font;
    }

    public void setFontColor(HtmlColor color) {
        font.setColor(color);
    }

    public void setFontSize(double d) {
        font.setSize(d);
    }

    public void setFontFamily(String family) {
        font.setFamily(family);
    }

    public void setFontWeight(int weight) {
        font.setWeight(weight);
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    @Override
    public boolean hit(double x, double y) {
        return false;
    }

    public void setFocused(boolean focused) {
        this.focused = focused;
    }

    public boolean isFocused() {
        return focused;
    }

    public void setVisible(boolean visible) {
        this.visible = visible;
    }

    public boolean isVisible() {
        return visible;
    }

    public String getMessage(double x, double y) {
        return "";
    }

}
