package gpsclock;

/**
 * Duplicate another line, exactly.
 */
public class DuplicateLineProgram extends RelativeLineProgram {
	protected DuplicateLineProgram(int source) {
		super(source);
	}

	@Override
	protected int[] encodeRelative() {
		return new int[0];
	}
}
