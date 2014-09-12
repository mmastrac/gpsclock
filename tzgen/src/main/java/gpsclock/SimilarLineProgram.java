package gpsclock;

import java.util.ArrayList;
import java.util.Arrays;

/**
 * Like {@link DiffLineProgram}, but stores only the per-space diffs and doesn't add/remove any items.
 */
public class SimilarLineProgram extends RelativeLineProgram {
	private int[] diffs;

	public SimilarLineProgram(ImageProgram imageProgram, int source, int[] diffs) {
		super(imageProgram, source);
		this.diffs = diffs;
	}

	@Override
	protected LineSpec computeInternal(LineSpec spec) {
		LineSpec lineSpec = spec.clone();
		ArrayList<int[]> data = lineSpec.getSpec();
		for (int i = 0; i < data.size(); i++) {
			data.get(i)[0] += diffs[i];
		}

		return lineSpec;
	}

	@Override
	protected int[] encodeRelative() {
		return diffs;
	};
	
	@Override
	public int opcode() {
		return 3;
	}
	
	@Override
	public String toString() {
		return "SIMILAR " + source + " " + Arrays.toString(diffs);
	}
}
