package gpsclock;

/**
 * Exactly-specified line program.
 */
public class FullLineProgram extends LineProgram {
	private LineSpec lineSpec;

	public FullLineProgram(LineSpec lineSpec) {
		this.lineSpec = lineSpec;
	}
	
	@Override
	public int[] encode() {
		return lineSpec.encode();
	}
}
