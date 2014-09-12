package gpsclock;

import java.util.Arrays;

import org.junit.Test;

public class IntListPackerTest {
	@Test
	public void testSimpleList() {
		byte[] data = IntListPacker.pack(new int[] { 1, 2, 3, 4, 5 });
		System.out.println(Arrays.toString(data));
	}

	@Test
	public void testZeros() {
		byte[] data = IntListPacker.pack(new int[] { 0, 0, 0 });
		System.out.println(Arrays.toString(data));
	}

	@Test
	public void testLargeInt() {
		byte[] data = IntListPacker.pack(new int[] { 1024, 512, 256 });
		System.out.println(Arrays.toString(data));
	}

	@Test
	public void testRealSequence() {
		int[] in = new int[] { 399, 899, 99, 799, 199, 599, 422, 29, 22, 21, 1,
				99, 1899, 1299, 899, 1499, 299, 1199, 299, 365, 370, 324, 142,
				108, 307, 138, 224, 138, 224, 138, 224, 40, 111, 235, 290, 139,
				35, 365 };
		byte[] data = IntListPacker.pack(in);
		System.out.println(Arrays.toString(data));
		System.out.println(in.length + " -> " + data.length);
	}

	@Test
	public void testRealSequence2() {
		int[] in = new int[] { 399, 99, 799, 99, 799, 199, 502, 496, 99, 1307,
				12, 678, 1299, 899, 1499, 299, 1199, 299, 365, 134, 370, 324,
				142, 108, 307, 138, 306, 40, 16, 40, 111, 235, 290, 139, 35,
				365 };
		byte[] data = IntListPacker.pack(in);
		System.out.println(Arrays.toString(data));
		System.out.println(in.length + " -> " + data.length);
	}
}
