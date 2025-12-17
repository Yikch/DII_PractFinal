package alertador.videoportero.telegram.mqtt;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonMappingException;

import java.util.concurrent.CompletionStage;

import org.eclipse.microprofile.reactive.messaging.Incoming;

import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import io.smallrye.reactive.messaging.mqtt.ReceivingMqttMessageMetadata;
import alertador.videoportero.telegram.services.ImageAlertService;

import org.eclipse.microprofile.reactive.messaging.Message;
import org.jboss.logging.Logger;

@ApplicationScoped
public class ImageAlertVideoreporter {

    private ImageAlertService imageAlertService;
    private Logger log;

    @Inject
    public ImageAlertVideoreporter(ImageAlertService imageAlertService, Logger log) {
        this.imageAlertService = imageAlertService;
        this.log = log;
    }

    @Incoming("videoreportero")
    public CompletionStage<Void> processImageVideoporterMqtt(Message<byte []> file) throws JsonProcessingException, JsonMappingException {  
        log.infof("Executing method %s", "processImageVideoporterMqtt");
        log.info("Processing metadata");
        try {
            file.getMetadata(ReceivingMqttMessageMetadata.class).ifPresent(metadata -> {
                log.info("Metadata processed");
                log.debugf("Metadata topic: %s", metadata.getTopic());
                this.imageAlertService.processImageVideoporterMqtt(metadata.getTopic(), file.getPayload());
            });
        } catch (Exception e) {
            log.errorf("Error: %s", e.getMessage());
            return file.nack(e);
        }
        return file.ack();
    }
}