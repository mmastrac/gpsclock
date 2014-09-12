package gpsclock;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import com.sksamuel.diffpatch.DiffMatchPatch;
import com.sksamuel.diffpatch.DiffMatchPatch.Diff;
import com.sksamuel.diffpatch.DiffMatchPatch.Operation;

public class LineSpec implements Cloneable {
	private ArrayList<int[]> spec;
	
	public LineSpec(ArrayList<int[]> spec) {
		this.spec = spec;
	}
	
	public ArrayList<int[]> getSpec() {
		return spec;
	}

	public int[] encode() {
		int[] encoded = new int[spec.size() * 2];
		// Encode counts, then colors
		for (int i = 0; i < spec.size(); i++) {
			encoded[i] = spec.get(i)[0];
			encoded[i + spec.size()] = spec.get(i)[1];
		}
		return encoded;
	}
	
	@SuppressWarnings("unchecked")
	@Override
	protected LineSpec clone() {
		ArrayList<int[]> clonedList = (ArrayList<int[]>) spec.clone();
		for (int i = 0; i < clonedList.size(); i++)
			clonedList.set(i, clonedList.get(i).clone());
		LineSpec lineSpec = new LineSpec(clonedList);
		return lineSpec;
	}
	
	@Override
	public int hashCode() {
		int hash = this.spec.size();
		for (int i = 0; i < this.spec.size(); i++) {
			hash = hash << 2;
			hash ^= this.spec.get(i)[0] * 7;
			hash = hash << 2;
			hash ^= this.spec.get(i)[1] * 3;
		}
		
		return hash;
	}
	
	@Override
	public boolean equals(Object obj) {
		LineSpec other = (LineSpec) obj;
		if (other.spec.size() != this.spec.size())
			return false;
		
		for (int i = 0; i < this.spec.size(); i++) {
			int[] a1 = this.spec.get(i);
			int[] a2 = other.spec.get(i);
			if (a1[0] != a2[0])
				return false;
			if (a1[1] != a2[1])
				return false;
		}
		
		return true;
	}
	
	/**
	 * Returns a diff that gets you to this {@link LineSpec} from the other one (colors only).
	 */
	public List<LineSpecDiff> diffColorsToThis(LineSpec other) {
		String otherString = other.diffString();
		String thisString = diffString();
		
		DiffMatchPatch dmp = new DiffMatchPatch();
		LinkedList<Diff> diffs = dmp.diff_main(otherString, thisString, false);
		dmp.diff_cleanupEfficiency(diffs);
		
		ArrayList<LineSpecDiff> lineDiff = new ArrayList<>(diffs.size());
		int thisIndex = 0;
		int otherIndex = 0;
		
		for (Diff diff : diffs) {
			LineSpecDiff lineSpecDiff = new LineSpecDiff();
			int len = diff.text.length();
			if (diff.operation == Operation.EQUAL) {
				boolean identical = true;
				for (int i = 0; i < len; i++) {
					if (this.spec.get(thisIndex + i)[0] != other.spec.get(otherIndex + i)[0]) {
						identical = false;
						break;
					}
				}
				
				if (identical) {
					lineSpecDiff.operation = LineSpecDiff.Operation.EQUAL;
					lineSpecDiff.data = new int[] { len };
				} else {
					lineSpecDiff.operation = LineSpecDiff.Operation.SIMILAR;
					lineSpecDiff.data = subtract(other, thisIndex, otherIndex, len);
				}
				
				thisIndex += len;
				otherIndex += len;
			} else if (diff.operation == Operation.DELETE) {
				lineSpecDiff.operation = LineSpecDiff.Operation.DELETE;
				lineSpecDiff.data = new int[] { len };
				otherIndex += len;
			} else if (diff.operation == Operation.INSERT) {
				lineSpecDiff.operation = LineSpecDiff.Operation.INSERT;
				lineSpecDiff.data = new int[len * 2];
				for (int i = 0; i < len; i++) {
					lineSpecDiff.data[i * 2] = this.spec.get(thisIndex)[0];
					lineSpecDiff.data[i * 2 + 1] = this.spec.get(thisIndex)[1];
					thisIndex++;
				}
			}

			lineDiff.add(lineSpecDiff);
		}
		
		return lineDiff;
	}

	private String diffString() {
		String diff = "";
		for (int i = 0; i < this.spec.size(); i++) {
			diff += (char)this.spec.get(i)[1];
		}
		
		return diff;
	}

	public int[] subtract(LineSpec other) {
		if (this.spec.size() != other.spec.size())
			throw new IllegalArgumentException("LineSpecs must be the same size");
		
		return subtract(other, 0, 0, this.spec.size());
	}

	public int[] subtract(LineSpec other, int thisOffset, int otherOffset, int length) {
		int[] ret = new int[length];
		
		for (int i = 0; i < length; i++) {
			int[] a1 = this.spec.get(i + thisOffset);
			int[] a2 = other.spec.get(i + otherOffset);
			
			if (a1[1] != a1[1])
				throw new IllegalArgumentException("LineSpecs must contain the same colors");
			
			ret[i] = a1[0] - a2[0];
		}
		
		return ret;
	}
	
	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();
		for (int[] pairs : spec) {
			builder.append("[" + pairs[0] + "," + pairs[1] + "]");
		}
		
		return builder.toString();
	}
}
