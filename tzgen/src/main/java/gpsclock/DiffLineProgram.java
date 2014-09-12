package gpsclock;

import java.util.List;

/**
 * A diff from another line program.
 */
public class DiffLineProgram extends RelativeLineProgram {
	private List<LineSpecDiff> diffs;

	protected DiffLineProgram(ImageProgram imageProgram, int source,
			List<LineSpecDiff> diffs) {
		super(imageProgram, source);
		this.diffs = diffs;
	}

	@Override
	protected int[] encodeRelative() {
		int count = 0;
		for (LineSpecDiff diff : diffs) {
			count++; // op
			if (diff.data != null)
				count += 1 + diff.data.length; // 1 + array contents
		}

		int[] data = new int[count];
		int index = 0;
		for (LineSpecDiff diff : diffs) {
			data[index++] = diff.operation.ordinal();
			switch (diff.operation) {
			case DELETE:
			case EQUAL:
				// These ops have only a single value
				data[index++] = diff.data[0];
				break;
			case INSERT:
			case SIMILAR:
				// These ops use an array
				data[index++] = diff.data.length;
				for (int i = 0; i < diff.data.length; i++) {
					data[index++] = diff.data[i];
				}
				break;
			}
		}

		return data;
	}

	@Override
	protected LineSpec computeInternal(LineSpec source) {
		return LineSpecDiff.apply(diffs, source);
	}
	
	@Override
	public int opcode() {
		return 1;
	}
	
	@Override
	public String toString() {
		return "DIFF " + source + " " + diffs;
	}
}
