package gpsclock;

import org.junit.Test;
import static org.junit.Assert.*;

public class BitStreamTest {
	@Test
	public void testUnsignedInt() {
		BitStream b = new BitStream();
		b.addUnsignedInt(1, 4);
		assertEquals(1, b.getUnsignedInt(0, 4));
	}
}
