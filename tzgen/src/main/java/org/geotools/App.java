package org.geotools;

import gnu.trove.map.hash.TIntIntHashMap;
import gnu.trove.set.hash.TIntHashSet;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.image.BufferedImage;
import java.io.File;

import javax.imageio.ImageIO;

import org.geotools.data.FileDataStore;
import org.geotools.data.FileDataStoreFinder;
import org.geotools.data.simple.SimpleFeatureSource;
import org.geotools.feature.FeatureIterator;
import org.opengis.feature.Property;
import org.opengis.feature.simple.SimpleFeature;

import com.vividsolutions.jts.awt.ShapeWriter;
import com.vividsolutions.jts.geom.Geometry;

/**
 * Prompts the user for a shapefile and displays the contents on the screen in a
 * map frame.
 * <p>
 * This is the GeoTools Quickstart application used in documentationa and
 * tutorials. *
 */
public class App {

	/**
	 * GeoTools Quickstart demo application. Prompts the user for a shapefile
	 * and displays its contents on the screen in a map frame
	 */
	public static void main(String[] args) throws Exception {
		// display a data store file chooser dialog for shapefiles
		File file = new File("/tmp/world/tz_world.shp");

		FileDataStore store = FileDataStoreFinder.getDataStore(file);
		SimpleFeatureSource featureSource = store.getFeatureSource();

		FeatureIterator<SimpleFeature> it = featureSource.getFeatures()
				.features();

		Rectangle imageBounds = new Rectangle(11000, 5400);
		BufferedImage image = new BufferedImage(imageBounds.width,
				imageBounds.height, BufferedImage.TYPE_INT_RGB);
		Graphics2D gr = image.createGraphics();
		gr.setPaint(Color.WHITE);
		gr.fill(imageBounds);
		gr.setPaint(Color.BLACK);

		int count = 0;
		ShapeWriter shapeWriter = new ShapeWriter((src, dest) -> {
			dest.setLocation((src.x + 180) / 360 * imageBounds.width,
					(1 - (src.y + 90) / 180) * imageBounds.height);
		});
		while (it.hasNext()) {
			SimpleFeature f = it.next();
			Shape shape = shapeWriter
					.toShape((Geometry) f.getDefaultGeometry());
			Property x = f.getProperty("TZID");
			String tz = (String) x.getValue();
			gr.setPaint(new Color(tz.hashCode(), false));
			gr.fill(shape);
			count++;
		}

		ImageIO.write(image, "png", new File("/tmp/world.png"));
		
		int maxColorCount = 0;
		TIntIntHashMap countHistogram = new TIntIntHashMap();
		for (int y = 0; y < imageBounds.height; y++) {
			int colorCount = 0;
			int colorTransitions = 0;
			
			int lastColor = -2;
			TIntHashSet colors = new TIntHashSet();
			for (int x = 0; x < imageBounds.width; x++) {
				int c = image.getRGB(x, y);
				if (colors.add(c)) {
					colorCount++;
				}
				if (lastColor != c) {
					colorTransitions++;
					lastColor = c;
				}
			}
			
			if (colorTransitions > maxColorCount)
				maxColorCount = colorTransitions;
			
			countHistogram.adjustOrPutValue(colorTransitions, 1, 1);
		}

		for (int i = 0; i < maxColorCount + 1; i++) {
			System.out.println(i + "\t" + countHistogram.get(i));
		}
	}

}
