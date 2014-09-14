package gpsclock;

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
 * - Lists of zeros - Lists of integers that are in a similar range
 */
public class IntListPacker {
	private static final int[] SMALL_NUMBER_HEADER = new int[] { 0, 0 };
	private static final int[] MEDIUM_NUMBER_HEADER = new int[] { 0, 1 };
	private static final int[] LARGE_NUMBER_HEADER = new int[] { 1, 1 };
	
	private static final int[] ZERO_SEQUENCE_HEADER = new int[] { 0, 0, 0, 0, 0, 0 };
	private static final int[] FIXED_WIDTH_SEQUENCE_HEADER = new int[] { 1, 0, 0 };
	private static final int[] OFFSET_SEQUENCE_HEADER = new int[] { 1, 0, 1 };
	
	static File corpus = new File("/tmp/corpus.txt");
	static {
		corpus.delete();
	}

	// Encoding tags:
	// 000000xxx: 0s sequence, xxx encodes 1-9
	// 00xxxx: 1 -> 15ww
	// 01xxxxxxxx: 16->261
	// 11sxxxxxx xxxxxx xx: large number (s = sign, if s == 0, add 262 to value)
	// 101(offset)(length)xxxxxxxx...: offset sequence, length encodes 2-10
	// (each further number is regularly-encoded)
	// 100(width=4)(length)s(xxx): width-constrained sequence (width=2-17),
	// length encodes 3-11, s = sign
	public static byte[] pack(int[] ints) {
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
//				System.out.println("small: " + v);
				bits.add(SMALL_NUMBER_HEADER);
				bits.addUnsignedInt(v, 4);
			} else if (v > 16 && v < 256 + 16) {
//				System.out.println("medium: " + v);
				bits.add(MEDIUM_NUMBER_HEADER);
				bits.addUnsignedInt(v - 16, 8);
			} else {
//				System.out.println("large: " + v);
				bits.add(LARGE_NUMBER_HEADER);
				bits.add(v < 0);
				bits.addUnsignedInt(Math.abs(v), 14);
			}
			
			i++;
		}

		return bits.toByteArray();
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

//		System.out.println("zero sequence: " + i);
		bits.add(ZERO_SEQUENCE_HEADER);
		bits.addUnsignedInt(i - 1, 3);
		
		return index + i;
	}

	private static int fixedWidthSequence(BitStream bits, int[] ints, int index) {
		// Not enough space to bother trying
		if (ints.length - index < 3)
			return -1;

		int width = Integer.highestOneBit(ints[index]);
		// If too narrow, just bump it up to 2
		if (width < 2)
			width = 2;
		// Too wide for this encoding?
		if (width > 17)
			return -1;

		int i;
		boolean signed = false;

		// Attempt to find the most optimal fixed-width sequence, up to 11 bytes
		for (i = 0; i < 11 && index + i < ints.length; i++) {
			// TODO: make this smarter. for now it's just greedy and we'll just
			// look for the first byte that would waste two zeros.

			int n = ints[index + i];
			if (n < 0) {
				n = Math.abs(n);
				signed = true;
			}
			boolean ok = Integer.highestOneBit(n) >= width - 2;

			// Too short, just bail
			if (!ok) {
				if (i < 3)
					return -1;

				i--;
				break;
			}
		}

//		System.out.println("offset sequence: width = " + width + ", length = " + i);

		bits.add(FIXED_WIDTH_SEQUENCE_HEADER);
		bits.add(signed);

		bits.addUnsignedInt(width - 2, 4);
		bits.addUnsignedInt(i - 3, 3);

		for (int j = 0; j < i; j++) {
			int n = ints[index + j];
			if (signed) {
				bits.add(n < 0);
			}
			bits.addUnsignedInt(n, width);
		}
		
		return index + i + 1;
	}

	public static int[] unpack(byte[] bytes) {
		return null;
	}
}
