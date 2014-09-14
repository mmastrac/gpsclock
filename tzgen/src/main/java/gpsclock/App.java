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
import java.io.Writer;
import java.util.ArrayList;

import javax.imageio.ImageIO;

import org.geotools.data.FileDataStore;
import org.geotools.data.FileDataStoreFinder;
import org.geotools.data.simple.SimpleFeatureSource;
import org.geotools.feature.FeatureIterator;
import org.opengis.feature.Property;
import org.opengis.feature.simple.SimpleFeature;

import com.vividsolutions.jts.awt.PointTransformation;
import com.vividsolutions.jts.awt.ShapeWriter;
import com.vividsolutions.jts.geom.Geometry;
import com.vividsolutions.jts.geom.Point;

public class App {
	public static void main(String[] args) throws Exception {
		File file = new File("/tmp/world/tz_world.shp");
		File output = new File("/tmp/output.dat");
		File outputImage = new File("/tmp/world.png");

		FileDataStore store = FileDataStoreFinder.getDataStore(file);
		SimpleFeatureSource featureSource = store.getFeatureSource();

		Rectangle imageBounds;
		BufferedImage image;

		TIntIntHashMap colorIds = computeColorIds(featureSource);

//		outputImage.delete();
		if (outputImage.exists()) {
			System.out.println("Reading cached image...");
			image = ImageIO.read(outputImage);
			imageBounds = new Rectangle(image.getWidth(), image.getHeight());
		} else {
			imageBounds = new Rectangle(11000, 5400);
			image = createImage(imageBounds);

			PointTransformation pointTransformer = (src, dest) -> {
				dest.setLocation((src.x + 180) / 360 * imageBounds.width,
						(1 - (src.y + 90) / 180) * imageBounds.height);
			};

			computeVoroni(featureSource, image, pointTransformer);
			drawShapes(featureSource, image, pointTransformer);

			ImageIO.write(image, "png", outputImage);
		}

		try (BufferedWriter w = new BufferedWriter(new OutputStreamWriter(
				new FileOutputStream(output)))) {
			computeTransitions(imageBounds, image, colorIds, w);
		}
	}

	private static TIntIntHashMap computeColorIds(
			SimpleFeatureSource featureSource) throws IOException {
		TIntIntHashMap ids = new TIntIntHashMap();
		TIntObjectHashMap<String> hashes = new TIntObjectHashMap<>();
		ids.put(0xffffffff, 0);
		FeatureIterator<SimpleFeature> it = featureSource.getFeatures()
				.features();
		while (it.hasNext()) {
			SimpleFeature f = it.next();
			String tz = getFeatureTZ(f);
			int c = getFeatureColor(f);
			String r = hashes.putIfAbsent(c, tz);
			if (r != null && !r.equals(tz)) {
				throw new RuntimeException("Duplicate hash detected: " + c
						+ " " + hashes.get(c) + " " + tz);
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
		System.out.println("Drawing shapes...");

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

		int skip = 1;
		int halfSkip = skip / 2;
		int[][] seedArray = seeds.keySet().toArray(new int[seeds.size()][]);
		int seedCount = seedArray.length;
		Color[] colors = new Color[seedCount];
		for (int i = 0; i < colors.length; i++) {
			colors[i] = new Color(seeds.get(seedArray[i]), false);
		}

		int w = imageBounds.width;
		int h = imageBounds.height;
		for (int x = 0; x < w; x += skip) {
			for (int y = 0; y < h; y += skip) {
				long minDist = Long.MAX_VALUE;
				int closest = -1;
				for (int i = 0; i < seedCount; i++) {
					int[] seed = seedArray[i];
					int xp = x + halfSkip - seed[0];
					int yp = y + halfSkip - seed[1];
					long dist = xp * xp + yp * yp;
					if (dist < minDist) {
						closest = i;
						minDist = dist;
					}
				}

				// image.setRGB(x, y, 0);
				gr.setPaint(colors[closest]);
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
			BufferedImage image, TIntIntHashMap colorIds, BufferedWriter w)
			throws IOException {
		System.out.println("Computing transitions...");

		ImageProgram imageProgram = new ImageProgram();

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
						transitions.add(new int[] { count,
								colorIds.get(lastColor) });
					}
					lastColor = c;
					count = 0;
				} else {
					count++;
				}
			}

			transitions.add(new int[] { count, colorIds.get(lastColor) });

			imageProgram.add(new FullLineProgram(imageProgram, new LineSpec(
					transitions)));
		}

		imageProgram.optimize();
		byte[] program = imageProgram.save();
		File f = new File("/tmp/image.bin");
		try (FileOutputStream os = new FileOutputStream(f)) {
			os.write(program);
		}

		f = new File("/tmp/image.txt");
		try (Writer writer = new OutputStreamWriter(new FileOutputStream(f))) {
			writer.write(imageProgram.dump());
		}
	}

}
