package gpsclock;

/**
 * Exactly-specified line program.
 */
public class FullLineProgram extends LineProgram {
	private LineSpec lineSpec;

	public FullLineProgram(ImageProgram imageProgram, LineSpec lineSpec) {
		super(imageProgram);
		this.lineSpec = lineSpec;
	}
	
	@Override
	public int[] encodeData() {
		return lineSpec.encode();
	}

	public LineSpec getSpec() {
		return lineSpec;
	}
	
	@Override
	public LineSpec compute() {
		return lineSpec;
	}
	
	@Override
	public int opcode() {
		return 0;
	}
	
	@Override
	public String toString() {
		return "FULL " + lineSpec;
	}
}
