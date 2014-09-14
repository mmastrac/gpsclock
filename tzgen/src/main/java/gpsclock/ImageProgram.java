package gpsclock;

import gpsclock.LineSpecDiff.Operation;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.google.protobuf.ByteString;

public class ImageProgram implements Iterable<LineProgram> {
	private ArrayList<LineProgram> lines = new ArrayList<>();

	public ImageProgram() {
	}

	public void add(LineProgram program) {
		lines.add(program);
	}

	public void set(int index, LineProgram newProgram) {
		if (get(index).compute().hashCode() != newProgram.compute().hashCode()) {
			throw new IllegalArgumentException(
					"Cannot replace a program entry with one that does not result in the same data");
		}

		lines.set(index, newProgram);
	}

	public LineProgram get(int index) {
		return lines.get(index);
	}

	public int hashCode() {
		int hash = 0;
		for (LineProgram program : lines) {
			hash += 0xdede;
			hash ^= program.compute().hashCode();
		}
		return hash;
	}

	public int cost() {
		int cost = 0;
		for (LineProgram program : lines) {
			cost += program.cost();
		}
		return cost;
	}

	@Override
	public Iterator<LineProgram> iterator() {
		return lines.iterator();
	}

	/**
	 * Optimizes this image program by replacing {@link LineProgram}s with more
	 * efficient ones.
	 * 
	 * If two lines are identical, we use {@link DuplicateLineProgram}. If two
	 * lines share the same colors, but not counts, we use
	 * {@link SimilarLineProgram}. Otherwise we use a {@link DiffLineProgram}.
	 */
	public void optimize() {
		int initialCost = cost();
		System.out.println("Initial cost = " + initialCost / 8);
		int hashCode = hashCode();
		System.out.println("Initial hash = " + hashCode());

		for (int i = 1; i < lines.size(); i++) {
			LineSpec thisLine = lines.get(i).compute();

			// The initial candidate is a full encoding
			LineProgram lowestCostProgram = new FullLineProgram(this, thisLine);
			int minCost = lowestCostProgram.cost();

			// If the previous line has a depth of 50, let's take that as a hint
			// that we need a fully-encoded line
			if (get(i - 1).depth() < 50) {
				// Compute the lowest cost encoding over a window of the last 50
				// lines
				for (int j = Math.max(0, i - 100); j < i; j++) {
					LineProgram refLineProgram = lines.get(j);
					int refIndex = j;
	
					// Dereference any dupe programs
					while (refLineProgram instanceof DuplicateLineProgram) {
						refIndex = ((DuplicateLineProgram) refLineProgram).source();
						refLineProgram = get(refIndex);
					}
	
					// We don't want to consider reference chains this big
					if (refLineProgram.depth() > 50) {
						// System.out.println("Bailing on a too-long reference chain");
						continue;
					}
	
					LineSpec refLine = refLineProgram.compute();
					if (thisLine.equals(refLine)) {
						lowestCostProgram = new DuplicateLineProgram(this, refIndex);
						break;
					}
	
					LineProgram candidate = computeLowestCostProgram(thisLine,
							refLine, refIndex);
					
					// Choose this candidate if it's the lowest cost, or the same
					// cost and a lower depth
					if (candidate.cost() < minCost || candidate.cost() == minCost
							&& candidate.depth() < lowestCostProgram.depth()) {
						lowestCostProgram = candidate;
						minCost = candidate.cost();
					}
				}
			}

			set(i, lowestCostProgram);

			if (hashCode() != hashCode) {
				System.out.println("hash changed");
			}
		}

		System.out.println();
		int dupes = 0, similar = 0, diffs = 0;
		for (int i = 0; i < lines.size(); i++) {
			if (get(i) instanceof DuplicateLineProgram)
				dupes++;
			if (get(i) instanceof SimilarLineProgram)
				similar++;
			if (get(i) instanceof DiffLineProgram)
				diffs++;
		}

		System.out.println("Dupes = " + dupes);
		System.out.println("Similar = " + similar);
		System.out.println("Diffs = " + diffs);

		int finalCost = cost();
		double pct = Math.round(((initialCost - finalCost)
				/ (double) initialCost * 10000))
				/ (double) 100;
		System.out.println("Final cost = " + finalCost / 8 + " " + pct + "%");
		System.out.println("Final hash = " + hashCode());
	}

	/**
	 * Persist this image program to storage.
	 */
	public byte[] save() {
		ByteString byteString = ByteString.EMPTY;

		// Probably not memory-efficient
		for (LineProgram lineProgram : lines) {
			byteString = byteString.concat(ByteString.copyFrom(IntListPacker
					.pack(lineProgram.encode()).toByteArray()));
		}

		return byteString.toByteArray();
	}

	private LineProgram computeLowestCostProgram(LineSpec thisLine,
			LineSpec lastLine, int lastLineIndex) {
		if (thisLine.equals(lastLine)) {
			return new DuplicateLineProgram(this, lastLineIndex);
		}

		List<LineSpecDiff> diff = thisLine.diffColorsToThis(lastLine);
		if (diff.size() == 1 && diff.get(0).operation == Operation.SIMILAR) {
			// Similar
			int[] offsets = diff.get(0).data;
			return new SimilarLineProgram(this, lastLineIndex, offsets);
		}

		DiffLineProgram diffLineProgram = new DiffLineProgram(this,
				lastLineIndex, diff);
		FullLineProgram fullLineProgram = new FullLineProgram(this, thisLine);

		if (diffLineProgram.cost() < fullLineProgram.cost())
			return diffLineProgram;

		return fullLineProgram;
	}

	public String dump() {
		StringBuilder builder = new StringBuilder();
		int index = 0;
		for (LineProgram lineProgram : lines) {
			builder.append(index++);
			builder.append('\t');
			builder.append(lineProgram);
			builder.append('\n');
		}
		return builder.toString();
	}
}
