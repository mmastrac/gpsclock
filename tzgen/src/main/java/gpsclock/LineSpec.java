package gpsclock;

import java.util.ArrayList;

public class LineSpec {
	private ArrayList<int[]> spec;
	
	public LineSpec(ArrayList<int[]> spec) {
		this.spec = spec;
	}
	
	public ArrayList<int[]> getSpec() {
		return spec;
	}

	public int[] encode() {
		int[] encoded = new int[spec.size() * 2];
		for (int i = 0; i < spec.size(); i++) {
			encoded[i * 2] = spec.get(i)[0];
			encoded[i * 2 + 1] = spec.get(i)[0];
		}
		return encoded;
	}
}
