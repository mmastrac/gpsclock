package gpsclock;

import java.util.BitSet;

public class BitStream {
	private BitSet bits;
	private int index;

	public BitStream() {
		bits = new BitSet();
	}

	public BitStream(byte[] bytes, int bitOffset, int bitLength) {
		bits = new BitSet(bitLength);
		int byteCursor = bitOffset >> 3;
		int bitCursor = bitOffset & 7;
		for (int i = 0; i < bitLength; i++) {
			add((bytes[byteCursor] & (1 << bitCursor)) != 0);
			bitCursor++;
			if (bitCursor == 8) {
				bitCursor = 0;
				byteCursor++;
			}
		}
	}

	public void add(int bit) {
		if (bit == 1)
			bits.set(index++);
		else if (bit == 0)
			index++;
		else
			throw new IllegalArgumentException("Bits are one or zero");
	}

	public void add(int[] bits) {
		for (int i : bits)
			add(i);
	}

	public byte[] toByteArray() {
		return bits.toByteArray();
	}

	public int size() {
		return index;
	}

	public void append(BitStream bs) {
		for (int i = 0; i < bs.size(); i++)
			add(bs.get(i));
	}

	public int get(int i) {
		return bits.get(i) ? 1 : 0;
	}

	public void addUnsignedInt(int n, int width) {
		for (int i = 0; i < width; i++) {
			add(n & 1);
			n >>= 1;
		}
		
		if (n != 0)
			throw new IllegalArgumentException("Tried to write a fatter int than width would allow: " + width);
	}

	public void add(boolean signed) {
		add(signed ? 1 : 0);
	}

	public int getUnsignedInt(int offset, int width) {
		int n = 0;
		for (int i = offset + width - 1; i >= offset; i--) {
			n <<= 1;
			n |= get(i);
		}
		return n;
	}
}
