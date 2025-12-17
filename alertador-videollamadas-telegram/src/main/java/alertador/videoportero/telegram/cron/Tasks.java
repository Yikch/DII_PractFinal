package alertador.videoportero.telegram.cron;

import alertador.videoportero.telegram.services.ImageAlertService;
import io.quarkus.scheduler.Scheduled;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;

@ApplicationScoped
public class Tasks {
    private ImageAlertService imageAlertService;

    @Inject
    public Tasks(ImageAlertService imageAlertService) {
        this.imageAlertService = imageAlertService;
    }

    @Scheduled(cron="${send.every.minute}")
    public void sendEveryMinute() {
        this.imageAlertService.sendNonSentPhotos();
    }

    @Scheduled(cron="${delete.sent}") 
    public void deleteSent() {
        this.imageAlertService.deleteSent();
    }
}