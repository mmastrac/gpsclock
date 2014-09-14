package gpsclock;

import gnu.trove.list.array.TIntArrayList;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.Arrays;

/**
 * Things we want to encode efficiently:
 * 
 * - Lists of zeros
 * 
 * - Lists of integers that are in a similar range
 */
public class IntListPacker {
	private static final int[] SMALL_NUMBER_HEADER = new int[] { 0, 0 };
	private static final int[] MEDIUM_NUMBER_HEADER = new int[] { 0, 1 };
	private static final int[] LARGE_NUMBER_HEADER = new int[] { 1, 1 };

	private static final int[] ZERO_SEQUENCE_HEADER = new int[] { 0, 0, 0, 0,
			0, 0 };
	private static final int[] FIXED_WIDTH_SEQUENCE_HEADER = new int[] { 1, 0,
			0 };
	private static final int[] OFFSET_SEQUENCE_HEADER = new int[] { 1, 0, 1 };

	static File corpus = new File("/tmp/corpus.txt");
	static {
		corpus.delete();
	}

	// Encoding tags:
	// 000000xxx: 0s sequence, xxx encodes 1-9
	// 00xxxx: 1 -> 15
	// 01xxxxxxxx: 16->261
	// 11sxxxxxx xxxxxx xx: large number (s = sign, if s == 0, add 262 to value)
	// 101(offset)(length)xxxxxxxx...: offset sequence, length encodes 2-10
	// (each further number is regularly-encoded)
	// 100(width=4)(length)s(xxx): width-constrained sequence (width=2-17),
	// length encodes 3-11, s = sign
	public static BitStream pack(int[] ints) {
		if (corpus != null) {
			try (Writer w = new OutputStreamWriter(new FileOutputStream(corpus,
					true))) {
				w.write(Arrays.toString(ints));
				w.write('\n');
			} catch (FileNotFoundException e) {
				throw new RuntimeException(e);
			} catch (IOException e) {
				throw new RuntimeException(e);
			}
		}

		BitStream bits = new BitStream();

		for (int i = 0; i < ints.length;) {
			int next = zeroSequence(bits, ints, i);
			if (next != -1) {
				i = next;
				continue;
			}

			next = offsetSequence(bits, ints, i);
			if (next != -1) {
				i = next;
				continue;
			}

			next = fixedWidthSequence(bits, ints, i);
			if (next != -1) {
				i = next;
				continue;
			}

			// direct encoding
			int v = ints[i];
			if (v < 16 && v > 0) {
				// System.out.println("small: " + v);
				bits.add(SMALL_NUMBER_HEADER);
				bits.addUnsignedInt(v, 4);
			} else if (v >= 16 && v < 256 + 16) {
				// System.out.println("medium: " + v);
				bits.add(MEDIUM_NUMBER_HEADER);
				bits.addUnsignedInt(v - 16, 8);
			} else {
				// System.out.println("large: " + v);
				bits.add(LARGE_NUMBER_HEADER);
				bits.add(v < 0);
				bits.addUnsignedInt(Math.abs(v), 14);
			}

			i++;
		}

		return bits;
	}

	private static int offsetSequence(BitStream bits, int[] ints, int i) {
		return -1;
	}

	private static int zeroSequence(BitStream bits, int[] ints, int index) {
		// No zeros!
		if (ints[index] != 0)
			return -1;

		int i;
		for (i = 1; i < 8 && index + i < ints.length; i++) {
			if (ints[index + i] != 0)
				break;
		}

		// System.out.println("zero sequence: " + i);
		bits.add(ZERO_SEQUENCE_HEADER);
		bits.addUnsignedInt(i - 1, 3);

		return index + i;
	}

	private static int fixedWidthSequence(BitStream bits, int[] ints, int index) {
		// Not enough space to bother trying
		if (ints.length - index < 3)
			return -1;

		int width = highestOneBitPos(Math.abs(ints[index]));
		// If too narrow, just bump it up to 2
		if (width < 2)
			width = 2;
		// Too wide for this encoding?
		if (width > 17)
			return -1;

		int count;
		boolean signed = ints[index] < 0;

		// Attempt to find the most optimal fixed-width sequence, up to 11 bytes
		for (count = 1; count < 11 && index + count < ints.length; count++) {
			// TODO: make this smarter. for now it's just greedy and we'll just
			// look for the first byte that would waste two zeros.

			int n = ints[index + count];
			if (n < 0) {
				n = Math.abs(n);
				signed = true;
			}
			int hi = highestOneBitPos(n);
			boolean ok = hi >= width - 2 && hi <= width;

			// Too short, just bail
			if (!ok) {
				if (count < 3)
					return -1;

				break;
			}
		}

		bits.add(FIXED_WIDTH_SEQUENCE_HEADER);
		bits.add(signed);

		bits.addUnsignedInt(width - 2, 4);
		bits.addUnsignedInt(count - 3, 3);

		for (int j = 0; j < count; j++) {
			int n = ints[index + j];
			if (signed) {
				bits.add(n < 0);
				n = Math.abs(n);
			}
			bits.addUnsignedInt(n, width);
		}

		return index + count;
	}

	// Hack
	private static int highestOneBitPos(int n) {
		int hi = Integer.highestOneBit(n);
		if (hi == 0)
			return 0;
		if (hi == 1)
			return 1;
		if (hi == 2)
			return 2;
		if (hi == 4)
			return 3;
		if (hi == 8)
			return 4;
		if (hi == 16)
			return 5;
		if (hi == 32)
			return 6;
		if (hi == 64)
			return 7;
		if (hi == 128)
			return 8;
		if (hi == 256)
			return 9;
		if (hi == 512)
			return 10;
		if (hi == 1024)
			return 11;
		if (hi == 2048)
			return 12;
		if (hi == 4096)
			return 13;
		
		throw new IllegalArgumentException("Too large");
	}

	public static int[] unpack(BitStream bits, int offset, int length) {
		TIntArrayList ints = new TIntArrayList(bits.size() / 8);
		while (offset < length) {
			int h1 = bits.get(offset++);
			int h2 = bits.get(offset++);
			
			if (h1 == SMALL_NUMBER_HEADER[0] && h2 == SMALL_NUMBER_HEADER[1]) {
				int n = bits.getUnsignedInt(offset, 4);
				offset += 4;
				if (n == 0) {
					int count = bits.getUnsignedInt(offset, 3) + 1;
					offset += 3;
					for (int i = 0; i < count; i++) {
						ints.add(0);
					}
				} else {
					ints.add(n);
				}
			} else if (h1 == MEDIUM_NUMBER_HEADER[0] && h2 == MEDIUM_NUMBER_HEADER[1]) {
				int n = bits.getUnsignedInt(offset, 8) + 16;
				offset += 8;
				ints.add(n);
			} else if (h1 == LARGE_NUMBER_HEADER[0] && h2 == LARGE_NUMBER_HEADER[1]) {
				int s = bits.get(offset++);
				int n = bits.getUnsignedInt(offset, 14);
				if (s == 1)
					n = -n;
				offset += 14;
				ints.add(n);
			} else if (h1 == FIXED_WIDTH_SEQUENCE_HEADER[0] && h2 == FIXED_WIDTH_SEQUENCE_HEADER[1]) {
				int h3 = bits.get(offset++);
				if (h3 == FIXED_WIDTH_SEQUENCE_HEADER[2]) {
					int signed = bits.get(offset++);
					int width = bits.getUnsignedInt(offset, 4) + 2;
					offset += 4;
					int count = bits.getUnsignedInt(offset, 3) + 3;
					offset += 3;
					
					for (int i = 0; i < count; i++) {
						int s = signed == 1 ? bits.get(offset++) : 0;
						int n = bits.getUnsignedInt(offset, width);
						offset += width;
						if (s == 1)
							n = -n;
						ints.add(n);
					}
				}
			}
		}
		
		return ints.toArray();
	}
}
