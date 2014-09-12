package gpsclock;

/**
 * Relatively-specified line program.
 */
public abstract class RelativeLineProgram extends LineProgram {
	protected int source;
	private LineSpec computed;

	protected RelativeLineProgram(ImageProgram imageProgram, int source) {
		super(imageProgram);
		this.source = source;
	}
	
	public int getSource() {
		return source;
	}
	
	public final LineSpec compute() {
		if (computed != null)
			return computed;
		
		return (computed = computeInternal(((FullLineProgram) imageProgram.get(source)).getSpec()));
	}
	
	/**
	 * Return the computed {@link LineSpec} for this.
	 */
	protected abstract LineSpec computeInternal(LineSpec source);
	
	public final int[] encodeData() {
		int[] data = encodeRelative();
		int[] output = new int[data.length + 1];
		output[0] = source;
		System.arraycopy(data, 0, output, 1, data.length);
		return output;
	}
	
	protected abstract int[] encodeRelative();
}
