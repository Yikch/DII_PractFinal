package alertador.videoportero.telegram.services;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import javax.imageio.IIOImage;
import javax.imageio.ImageIO;
import javax.imageio.ImageWriteParam;

import org.eclipse.microprofile.config.inject.ConfigProperty;
import org.jboss.logging.Logger;

import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;

@ApplicationScoped
public class CompressorService {
    @ConfigProperty(name="compressor.quality")
    private String quality;
    @ConfigProperty(name="compressor.format")
    private String format;
    private Logger log;

    @Inject
    public CompressorService(Logger log) {
        this.log = log;
    }

    public byte [] compress(byte [] image) {
        log.infof("Executing method %s", "compress");
        log.debugf("OG image size bytes: %s", image.length);
        byte [] result;
        try {
            var imagebuffer = ImageIO.read(new ByteArrayInputStream(image));
            var baos = new ByteArrayOutputStream();
            try (var ios = ImageIO.createImageOutputStream(baos)) {
                var writer = ImageIO.getImageWritersByFormatName(format).next();
                writer.setOutput(ios);
                
                var param = writer.getDefaultWriteParam();
                if (param.canWriteCompressed()) {
                    param.setCompressionMode(ImageWriteParam.MODE_EXPLICIT);
                    param.setCompressionQuality(Float.parseFloat(quality));
                }
                
                writer.write(null, new IIOImage(imagebuffer, null, null), param);
                writer.dispose();
            }
            result = baos.toByteArray();
            baos.close();
        } catch (IOException e) {
            log.errorf("Error at compressing: %s", e.getMessage());
            log.error("Returning original image");
            return image;
        }
        log.debugf("Compressed image size bytes: %s", result.length);
        return result;
    }

}
