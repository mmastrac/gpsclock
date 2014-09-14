package gpsclock;

/**
 * A line program can either be a fully-encoded line, or a line relative to
 * another line.
 */
public abstract class LineProgram {
	protected ImageProgram imageProgram;
	private int cost = -1;

	public LineProgram(ImageProgram imageProgram) {
		this.imageProgram = imageProgram;
	}

	public int cost() {
		if (this.cost != -1)
			return this.cost;
		
		int[] encoded = encodeData();
		BitStream packed = IntListPacker.pack(encoded);
		return (this.cost = packed.size());
	}

	public int fullCost() {
		if (this instanceof FullLineProgram)
			return cost();
		return (new FullLineProgram(imageProgram, compute())).cost();
	}
	
	public final int[] encode() {
		// TODO: we should probably pass an IntBuilder down into these methods
		
		// TODO: the opcode probably shouldn't be here as it's a fixed-width field we can optimize out
		int[] data = encodeData();
		int[] output = new int[data.length + 1];
		System.arraycopy(data, 0, output, 1, data.length);
		return output;
	}
	
	protected abstract int[] encodeData();

	public abstract LineSpec compute();
	
	public abstract int opcode();
	
	public abstract int depth();
}
