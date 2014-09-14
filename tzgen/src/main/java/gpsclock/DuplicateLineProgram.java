package gpsclock;

/**
 * Duplicate another line, exactly.
 */
public class DuplicateLineProgram extends RelativeLineProgram {
	protected DuplicateLineProgram(ImageProgram imageProgram, int source) {
		super(imageProgram, source);
	}

	@Override
	protected int[] encodeRelative() {
		return new int[0];
	}

	@Override
	protected LineSpec computeInternal(LineSpec lineSpec) {
		return lineSpec;
	}
	
	@Override
	public int opcode() {
		return 2;
	}
	
	@Override
	public String toString() {
		return "DUPE " + source;
	}

	public int source() {
		return source;
	}
}
