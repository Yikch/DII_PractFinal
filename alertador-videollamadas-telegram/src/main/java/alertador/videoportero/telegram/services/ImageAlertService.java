package alertador.videoportero.telegram.services;

import alertador.videoportero.telegram.repository.ImageAlertRepository;

import org.eclipse.microprofile.config.inject.ConfigProperty;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import alertador.videoportero.telegram.client.AzureEdgeInfoClient;
import alertador.videoportero.telegram.client.TelegramClient;
import alertador.videoportero.telegram.dto.ImageAlert;
import alertador.videoportero.telegram.dto.PhotoMessage;
import alertador.videoportero.telegram.dto.SyncedDevices;

import java.util.AbstractMap;
import java.util.Base64;
import java.util.List;

import org.eclipse.microprofile.rest.client.inject.RestClient;
import org.jboss.logging.Logger;

@ApplicationScoped
public class ImageAlertService {

    private static final Integer NON_SENT = 0;

    private ImageAlertRepository imageAlertRepository;
    private TelegramClient telegramClient;
    private AzureEdgeInfoClient azureEdgeInfoClient;
    private CompressorService compressorService;
    private Logger log;

    @ConfigProperty(name="mp.messaging.incoming.videoreportero.topic")
    private String brokerTopic;
    
    @Inject
    public ImageAlertService(
        ImageAlertRepository imageAlertRepository, 
        @RestClient TelegramClient telegramClient, 
        @RestClient AzureEdgeInfoClient azureEdgeInfoClient, 
        CompressorService compressorService, 
        Logger log
    ) {
        this.imageAlertRepository = imageAlertRepository;
        this.telegramClient = telegramClient;
        this.azureEdgeInfoClient = azureEdgeInfoClient;
        this.compressorService = compressorService;
        this.log = log;
    }

    public ImageAlert insert(ImageAlert imageAlert) {
        log.infof("Executing method %s", "insert");
        log.debugf("With parameters %s", imageAlert.toString());
        return this.imageAlertRepository.insert(imageAlert);
    }

    public List<ImageAlert> selectNonSent() {
        log.infof("Executing method %s", "selectNonSent");
        return this.imageAlertRepository.selectNonSent();
    }

    public void markToDelete(ImageAlert update) {
        log.infof("Executing method %s", "markToDelete");
        log.debugf("With parameters %s", update.toString());
        this.imageAlertRepository.update(new ImageAlert(update.id(), update.image(), update.topic(), update.telegramAddress(), 1));
    }

    public void deleteSent() {
        log.infof("Executing method %s", "deleteSent");
        this.imageAlertRepository.deleteSent();
    }

    public void sendNonSentPhotos() {
        log.infof("Executing method %s", "sendNonSentPhotos");
        try {
            var imageAlertsNonSent = selectNonSent();
            var chats = 
                telegramClient.getUpdates(-1).result().stream()
                .map(results -> results.message().chat())
                .toList();
            log.debugf("Images alerts non sent: %s, Chat ids: %s", imageAlertsNonSent, chats);
            imageAlertsNonSent.forEach(imageAlert -> {
                log.info("Checking images");
                var chatToSend = 
                    chats.stream()
                    .filter(chat -> chat.username().compareTo(imageAlert.telegramAddress()) == 0).toList();
                if (chatToSend.isEmpty()) {
                    return;
                }
                var photo = new PhotoMessage(chatToSend.get(0).id(), imageAlert.image(), "Se ha detectado a alguien que debe verificar");
                log.debugf("Sending photos: %s", photo);
                this.telegramClient.sendPhoto(photo);
                markToDelete(imageAlert);
            });
        } catch (Exception e) {
            log.errorf("Something went wrong during sending photos: %s. Not sent are still cached", e.getMessage());
            e.printStackTrace();
        } 
    }

    public void processImageVideoporterMqtt(String topic, byte[] payload) {
        log.infof("Executing method %s", "processImageVideoporterMqtt");
        log.debugf("Listened topic: %s", topic);
        var syncedDevices = azureEdgeInfoClient.syncDevices();
        log.debugf("Synced devices: %s", syncedDevices);
        log.debugf("Finding topic: %s, broker topic: %s", topic, brokerTopic);
        var foundTopic = 
            syncedDevices.stream()
            .map(device -> buildTopicWithTelegramAddress(device, brokerTopic))
            .filter(topicDevice -> topicDevice.getKey().compareTo(topic) == 0)
            .toList();
        log.debugf("Synced topics: %s", foundTopic);
        if (foundTopic.isEmpty()) {
            log.errorf("Unrecognized device from topic: %s", topic);
            return;
        }
        log.debugf("Recognized device from topic: %s", topic);
        var decoded = Base64.getDecoder().decode(payload);
        var imageAlertToInsert = new ImageAlert(null, decoded, foundTopic.get(0).getKey(), foundTopic.get(0).getValue(), NON_SENT);
        log.debugf("Image alert to insert: %s", imageAlertToInsert);
        insert(imageAlertToInsert);
    }

    private AbstractMap.SimpleEntry<String, String> buildTopicWithTelegramAddress(SyncedDevices device, String brokerTopic) {
        return 
            new AbstractMap.SimpleEntry<String, String>(
                brokerTopic.substring(0, brokerTopic.length()-1).concat(device.mqttToken()), 
                device.telegramAddress()
            );
    } 
}
