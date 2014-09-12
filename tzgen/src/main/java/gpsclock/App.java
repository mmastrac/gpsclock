package gpsclock;

import gnu.trove.map.hash.TIntIntHashMap;
import gnu.trove.map.hash.TIntObjectHashMap;
import gnu.trove.map.hash.TObjectIntHashMap;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.geom.Point2D;
import java.awt.image.BufferedImage;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.LinkedList;

import javax.imageio.ImageIO;

import org.geotools.data.FileDataStore;
import org.geotools.data.FileDataStoreFinder;
import org.geotools.data.simple.SimpleFeatureSource;
import org.geotools.feature.FeatureIterator;
import org.opengis.feature.Property;
import org.opengis.feature.simple.SimpleFeature;

import com.sksamuel.diffpatch.DiffMatchPatch;
import com.sksamuel.diffpatch.DiffMatchPatch.Diff;
import com.sksamuel.diffpatch.DiffMatchPatch.Operation;
import com.vividsolutions.jts.awt.PointTransformation;
import com.vividsolutions.jts.awt.ShapeWriter;
import com.vividsolutions.jts.geom.Geometry;
import com.vividsolutions.jts.geom.Point;

public class App {
	public static void main(String[] args) throws Exception {
		File file = new File("/tmp/world/tz_world.shp");
		File output = new File("/tmp/output.dat");

		FileDataStore store = FileDataStoreFinder.getDataStore(file);
		SimpleFeatureSource featureSource = store.getFeatureSource();

		Rectangle imageBounds = new Rectangle(11000, 5400);
		BufferedImage image = createImage(imageBounds);

		TIntIntHashMap colorIds = computeColorIds(featureSource);

		PointTransformation pointTransformer = (src, dest) -> {
			dest.setLocation((src.x + 180) / 360 * imageBounds.width,
					(1 - (src.y + 90) / 180) * imageBounds.height);
		};

		computeVoroni(featureSource, image, pointTransformer);
		drawShapes(featureSource, image, pointTransformer);

		ImageIO.write(image, "png", new File("/tmp/world.png"));

		try (BufferedWriter w = new BufferedWriter(new OutputStreamWriter(
				new FileOutputStream(output)))) {
			computeTransitions(imageBounds, image, colorIds, w);
		}
	}

	private static TIntIntHashMap computeColorIds(SimpleFeatureSource featureSource)
			throws IOException {
		TIntIntHashMap ids = new TIntIntHashMap();
		TIntObjectHashMap<String> hashes = new TIntObjectHashMap<>();
		FeatureIterator<SimpleFeature> it = featureSource.getFeatures()
				.features();
		while (it.hasNext()) {
			SimpleFeature f = it.next();
			String tz = getFeatureTZ(f);
			int c = getFeatureColor(f);
			String r = hashes.putIfAbsent(c, tz);
			if (r != null && !r.equals(tz)) {
				throw new RuntimeException("Duplicate hash detected: "
						+ c + " "
						+ hashes.get(c) + " " + tz);
			}

			if (!ids.containsKey(c)) {
				ids.put(c, ids.size());
			}
		}
		
		return ids;
	}

	private static BufferedImage createImage(Rectangle imageBounds) {
		BufferedImage image = new BufferedImage(imageBounds.width,
				imageBounds.height, BufferedImage.TYPE_INT_RGB);
		Graphics2D gr = image.createGraphics();
		gr.setPaint(Color.WHITE);
		gr.fill(imageBounds);
		gr.setPaint(Color.BLACK);
		return image;
	}

	private static void drawShapes(SimpleFeatureSource featureSource,
			BufferedImage image, PointTransformation pointTransformer)
			throws IOException {
		System.out.println("Drawing shapes");

		ShapeWriter shapeWriter = new ShapeWriter(pointTransformer);
		Graphics2D gr = image.createGraphics();

		FeatureIterator<SimpleFeature> it = featureSource.getFeatures()
				.features();
		while (it.hasNext()) {
			SimpleFeature f = it.next();
			Geometry geom = (Geometry) f.getDefaultGeometry();
			Shape shape = shapeWriter.toShape(geom);
			Color color = new Color(getFeatureColor(f), false);
			gr.setPaint(color);
			gr.fill(shape);
		}
	}

	private static void computeVoroni(SimpleFeatureSource featureSource,
			BufferedImage image, PointTransformation pointTransformer)
			throws IOException {
		Graphics2D gr = image.createGraphics();
		Rectangle imageBounds = new Rectangle(image.getWidth(),
				image.getHeight());

		System.out.println("Computing Voroni diagram from centroids...");

		TObjectIntHashMap<int[]> seeds = new TObjectIntHashMap<>();

		FeatureIterator<SimpleFeature> it = featureSource.getFeatures()
				.features();
		while (it.hasNext()) {
			SimpleFeature f = it.next();
			Geometry geom = (Geometry) f.getDefaultGeometry();
			Point c = geom.getCentroid();
			Point2D.Double dest = new Point2D.Double();
			pointTransformer.transform(c.getCoordinate(), dest);
			seeds.put(new int[] { (int) dest.x, (int) dest.y },
					getFeatureColor(f));
		}

		int skip = 25;
		for (int x = 0; x < imageBounds.width; x += skip) {
			for (int y = 0; y < imageBounds.height; y += skip) {
				long minDist = Long.MAX_VALUE;
				int[] closest = null;
				for (int[] seed : seeds.keySet()) {
					long dist = (x - seed[0]) * (x - seed[0]) + (y - seed[1])
							* (y - seed[1]);
					if (dist < minDist) {
						closest = seed;
						minDist = dist;
					}
				}

				gr.setPaint(new Color(seeds.get(closest), false));
				gr.fillRect(x, y, skip, skip);
			}

			if (x > 0) {
				if (x % 100 == 0)
					System.out.print(".");
				if (x % 1000 == 0)
					System.out.print(x);
			}
		}

		System.out.println();
	}

	private static int getFeatureColor(SimpleFeature f) {
		String tz = getFeatureTZ(f);
		int hashCode = tz.hashCode() | 0xff000000;
		return hashCode;
	}

	private static String getFeatureTZ(SimpleFeature f) {
		Property x = f.getProperty("TZID");
		String tz = (String) x.getValue();
		return tz;
	}

	private static void computeTransitions(Rectangle imageBounds,
			BufferedImage image, TIntIntHashMap colorIds, BufferedWriter w) throws IOException {
		System.out.println("Computing transitions...");
		
		int maxTransitions = 0;
		TIntIntHashMap countHistogram = new TIntIntHashMap();
		ArrayList<int[]> lastTransitions = new ArrayList<>();
		int dupes = 0, dupesCost = 0;
		int similar = 0, similarCost = 0;
		
		for (int y = 0; y < imageBounds.height; y++) {
			int lastColor = -1;
			int count = 0;
			TIntIntHashMap colors = new TIntIntHashMap();
			ArrayList<int[]> transitions = new ArrayList<>();
			for (int x = 0; x < imageBounds.width; x++) {
				int c = image.getRGB(x, y);
				if (!colors.containsKey(c)) {
					colors.put(c, colors.size());
				}
				if (lastColor != c) {
					if (count > 0) {
						transitions.add(new int[] { count, colorIds.get(lastColor) });
					}
					lastColor = c;
					count = 0;
				} else {
					count++;
				}
			}

			transitions.add(new int[] { count, colorIds.get(lastColor) });

			int colorTransitions = transitions.size();
			if (colorTransitions > maxTransitions)
				maxTransitions = colorTransitions;
			
			countHistogram.adjustOrPutValue(colorTransitions, 1, 1);

			String last = "";
			for (int[] transition : lastTransitions) {
				last += (char)transition[1];
			}
			String current = "";
			for (int[] transition : transitions) {
				current += (char)transition[1];
			}
			
			DiffMatchPatch dmp = new DiffMatchPatch();
			LinkedList<Diff> diffs = dmp.diff_main(last, current, false);
			dmp.diff_cleanupEfficiency(diffs);
			
			// Similar or dupe, so we can optimize this out here
			if (diffs.size() == 1 && diffs.get(0).operation == Operation.EQUAL) {
				String diff = "";
				boolean dupe = true;
				for (int i = 0; i < lastTransitions.size(); i++) {
					if (i > 0)
						diff += ",";
					int n = transitions.get(i)[0] - lastTransitions.get(i)[0];
					if (n != 0) {
						diff += n;
						dupe = false;
					}
				}
				
				if (dupe) {
					w.write("(dupe)\n");
					dupes++;
					dupesCost += transitions.size();
				} else {
					w.write("(similar)[");
					w.write(diff);
					w.write(']');
					w.write('\n');
					similar++;
					similarCost += transitions.size();
				}
				
				continue;
			}
			
			int lastIndex = 0, currentIndex = 0;
			for (Diff diff : diffs) {
				w.write("  " + diff.operation);
				w.write(' ');
				if (diff.operation == Operation.EQUAL) {
					for (int i = 0; i < diff.text.length(); i++) {
						int n = transitions.get(currentIndex)[0] - lastTransitions.get(lastIndex)[0];
						w.write("" + n);
						w.write(",");
						lastIndex++;
						currentIndex++;
					}
				} else if (diff.operation == Operation.DELETE) {
					w.write("count = " + diff.text.length());
					lastIndex += diff.text.length();
				} else if (diff.operation == Operation.INSERT) {
					for (int i = 0; i < diff.text.length(); i++) {
						w.write("[" + transitions.get(currentIndex)[0] + "," + transitions.get(currentIndex)[1] + "]");
						currentIndex++;
					}
				}
				w.write("\n");
			}

			lastTransitions = transitions;
			
			String line = "";
			for (int[] data : transitions) {
				line += "[" + data[0] + "," + data[1] + "]";
			}
			w.write(line);
			w.write('\n');
		}

		int cost = 0;
		for (int i = 0; i < maxTransitions + 1; i++) {
//			System.out.println(i + "\t" + countHistogram.get(i));
			cost += i * countHistogram.get(i);
		}

		System.out.println("Dupes = " + dupes + " (cost = " + dupesCost + ")");
		System.out.println("Similar = " + similar + " (cost = " + similarCost + ")");
		System.out.println("Cost = " + cost);
	}

}
