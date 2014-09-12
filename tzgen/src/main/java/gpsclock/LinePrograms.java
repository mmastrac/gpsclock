package gpsclock;

import java.util.ArrayList;
import java.util.Iterator;

public class LinePrograms implements Iterable<LineProgram> {
	private ArrayList<LineProgram> lines = new ArrayList<>();

	public LinePrograms() {
	}
	
	public void add(LineProgram program) {
		lines.add(program);
	}
	
	public void set(int index, LineProgram newProgram) {
		lines.set(index, newProgram);
	}
	
	public LineProgram get(int index) {
		return lines.get(index);
	}

	@Override
	public Iterator<LineProgram> iterator() {
		return lines.iterator();
	}
}
