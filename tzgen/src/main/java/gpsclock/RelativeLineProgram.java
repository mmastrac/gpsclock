package gpsclock;

/**
 * Relatively-specified line program.
 */
public abstract class RelativeLineProgram {
	protected int source;

	protected RelativeLineProgram(int source) {
		this.source = source;
	}
	
	public int getSource() {
		return source;
	}
	
	public int[] encode() {
		int[] data = encodeRelative();
		int[] output = new int[data.length + 1];
		output[0] = source;
		System.arraycopy(data, 0, output, 1, data.length);
		return output;
	}
	
	protected abstract int[] encodeRelative();
}
