package gpsclock;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class LineSpecDiff {
	public enum Operation {
		INSERT,
		DELETE,
		EQUAL,
		SIMILAR,
	}
	
    public Operation operation;
    public int[] data;
    
    public static LineSpec apply(List<LineSpecDiff> diffs, LineSpec source) {
		ArrayList<int[]> spec = new ArrayList<>();
		int index = 0;
		
		for (LineSpecDiff diff : diffs) {
			switch (diff.operation) {
			case EQUAL:
				for (int i = 0; i < diff.data[0]; i++) {
					spec.add(source.getSpec().get(index).clone());
					index++;
				}
				break;
			case SIMILAR:
				for (int i = 0; i < diff.data.length; i++) {
					int[] array = source.getSpec().get(index).clone();
					array[0] += diff.data[i];
					spec.add(array);
					
					index++;
				}
				break;
			case DELETE:
				index += diff.data[0];
				break;
			case INSERT:
				for (int i = 0; i < diff.data.length; i += 2) {
					spec.add(Arrays.copyOfRange(diff.data, i, i + 2));
				}
				break;
			}
		}
		
		return new LineSpec(spec);
    }
    
    @Override
    public String toString() {
    	return operation + " " + Arrays.toString(data);
    }
}
