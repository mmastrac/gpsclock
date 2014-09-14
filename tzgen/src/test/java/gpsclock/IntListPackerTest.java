package gpsclock;

import java.util.Arrays;

import org.junit.Test;

import static org.junit.Assert.*;

public class IntListPackerTest {
	@Test
	public void test1() {
		test(new int[] { 1 });
	}
	
	@Test
	public void testSmallInts() {
		for (int i = 2; i < 16; i++)
			test(new int[] { i });
	}
	
	@Test
	public void test16() {
		test(new int[] { 16 });
	}

	@Test
	public void testMediumInts() {
		for (int i = 17; i < 262; i++)
			test(new int[] { i });
	}

	@Test
	public void testSimpleList() {
		test(new int[] { 1, 2, 3, 4, 5 });
	}

	@Test
	public void testSimpleSignedList() {
		test(new int[] { 1, 2, -3, 4, -5 });
	}

	@Test
	public void testSimpleIntMultiSignList() {
		test(new int[] { 1, -1, 1, -1, 1, -1, 2, -3 });
	}

	@Test
	public void testZeros() {
		// Test zeros from length 1 all the way up to 20
		for (int i = 1; i < 20; i++) {
			int[] zeros = new int[i];
			test(zeros);
		}
	}

	@Test
	public void testLargeInts() {
		test(new int[] { 263 });
		test(new int[] { 1024 });
		test(new int[] { 512 });
		test(new int[] { 256 });
		test(new int[] { -10000 });
	}

	@Test
	public void testRealSequence() {
		int[] in = new int[] { 399, 899, 99, 799, 199, 599, 422, 29, 22, 21, 1,
				99, 1899, 1299, 899, 1499, 299, 1199, 299, 365, 370, 324, 142,
				108, 307, 138, 224, 138, 224, 138, 224, 40, 111, 235, 290, 139,
				35, 365 };
		test(in);
	}

	@Test
	public void testRealSequence2() {
		int[] in = new int[] { 399, 99, 799, 99, 799, 199, 502, 496, 99, 1307,
				12, 678, 1299, 899, 1499, 299, 1199, 299, 365, 134, 370, 324,
				142, 108, 307, 138, 306, 40, 16, 40, 111, 235, 290, 139, 35,
				365 };
		test(in);
	}

	private void test(int[] in) {
		BitStream out = IntListPacker.pack(in);
//		System.out.println(in.length + " -> " + ((out.size() + 1) / 8 + 1));
		int[] unpacked = IntListPacker.unpack(out, 0, out.size());
		assertEquals(Arrays.toString(in), Arrays.toString(unpacked));
	}
}
