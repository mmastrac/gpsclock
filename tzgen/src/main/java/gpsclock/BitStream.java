package gpsclock;

import java.util.BitSet;

public class BitStream {
	private BitSet bits;
	private int index;
	
	public BitStream() {
		bits = new BitSet();
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

	private int get(int i) {
		return bits.get(i) ? 1 : 0;
	}

	public void addUnsignedInt(int n, int width) {
		for (int i = 0; i < width; i++) {
			add(n & 1);
			n >>= 1;
		}
	}
}
