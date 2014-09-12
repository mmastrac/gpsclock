package gpsclock;

/**
 * A line program can either be a fully-encoded line, or a line relative to another line.
 */
public abstract class LineProgram {
	public abstract int[] encode();
}
