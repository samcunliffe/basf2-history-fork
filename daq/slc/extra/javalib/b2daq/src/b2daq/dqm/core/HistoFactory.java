package b2daq.dqm.core;

public class HistoFactory {
	
	static public Histo create(String type) {
		if (type.matches("H1C")) return new Histo1C();
		if (type.matches("H1S")) return new Histo1S();
		if (type.matches("H1I")) return new Histo1I();
		if (type.matches("H1F")) return new Histo1F();
		if (type.matches("H1D")) return new Histo1D();
		if (type.matches("H2C")) return new Histo2C();
		if (type.matches("H2S")) return new Histo2S();
		if (type.matches("H2I")) return new Histo2I();
		if (type.matches("H2F")) return new Histo2F();
		if (type.matches("H2D")) return new Histo2D();
		if (type.matches("G1C")) return new Graph1C();
		if (type.matches("G1S")) return new Graph1S();
		if (type.matches("G1I")) return new Graph1I();
		if (type.matches("G1F")) return new Graph1F();
		if (type.matches("G1D")) return new Graph1D();
		if (type.matches("TGC")) return new TimedGraph1C();
		if (type.matches("TGS")) return new TimedGraph1S();
		if (type.matches("TGI")) return new TimedGraph1I();
		if (type.matches("TGF")) return new TimedGraph1F();
		if (type.matches("TGD")) return new TimedGraph1D();
		
		return null;
	} 
}
