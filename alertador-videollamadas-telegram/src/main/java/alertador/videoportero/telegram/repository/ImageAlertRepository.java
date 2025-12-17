package alertador.videoportero.telegram.repository;

import alertador.videoportero.telegram.repository.core.CoreRepository;
import alertador.videoportero.telegram.repository.mapper.IdMapper;
import alertador.videoportero.telegram.repository.mapper.ImageAlertMapper;
import jakarta.annotation.PostConstruct;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.eclipse.microprofile.config.inject.ConfigProperty;

import java.util.List;

import org.jboss.logging.Logger;

import java.util.ArrayList;

import static alertador.videoportero.telegram.repository.sql.ImageAlertSql.*;

import alertador.videoportero.telegram.dto.ImageAlert;

import static alertador.videoportero.telegram.repository.core.CoreRepository.*;

@ApplicationScoped
public class ImageAlertRepository {
    @ConfigProperty(name="quarkus.datasource.tables.action")
    private String action;

    private CoreRepository coreRepository;
    private ImageAlertMapper imageAlertMapper;
    private IdMapper idMapper;
    private Logger log;

    private static final Integer NON_SENT = 0;
    private static final Integer SENT = 0;

    @Inject
    public ImageAlertRepository(CoreRepository coreRepository, ImageAlertMapper imageAlertMapper, IdMapper idMapper, Logger log) {
        this.coreRepository = coreRepository;
        this.imageAlertMapper = imageAlertMapper;
        this.idMapper = idMapper;
        this.log = log;
    }

    @PostConstruct
    private void checkTables() {
        List<Object> parameters = new ArrayList<>();
        parameters.add(TABLE_NAME);
        if (action.equals(DROP_CREATE_ACTION)) {
            log.infof("%s droped!", TABLE_NAME);
            this.coreRepository.execute(DROP_TABLE.concat(TABLE_NAME), new ArrayList<>());
        }
        if (!this.coreRepository.exist(CHECK_TABLE, parameters) && (action.equals(CREATE_ACTION) || action.equals(DROP_CREATE_ACTION))) {
            log.infof("%s created!", TABLE_NAME);
            this.coreRepository.execute(CREATE_TABLE, new ArrayList<>());
            return;
        }
        log.infof("%s not changed!", TABLE_NAME);
    }

    public ImageAlert insert(ImageAlert imageAlert) {
        log.infof("Executing method %s", "insert");
        log.debugf("With parameters: %s", imageAlert.toString());
        String sql = INSERT;
        List<Object> parameters = new ArrayList<>();
        parameters.add(imageAlert.image());
        parameters.add(imageAlert.topic());
        parameters.add(imageAlert.telegramAddress());
        parameters.add(imageAlert.sent());
        this.coreRepository.execute(sql, parameters);
        Integer id = idMapper.map(this.coreRepository.select(MAX_ID.concat(TABLE_NAME), new ArrayList<>(), 1));
        return new ImageAlert(id, imageAlert.image(), imageAlert.topic(), imageAlert.telegramAddress(), imageAlert.sent());
    }

    public List<ImageAlert> selectNonSent() {
        log.infof("Executing method %s", "selectNonSent");
        String sql = SELECT.concat(FROM).concat(WHERE_SENT);
        List<Object> parameters = new ArrayList<>();
        parameters.add(NON_SENT);
        return imageAlertMapper.map(this.coreRepository.select(sql, parameters, ROW_ELEMENTS));
    }

    public void update(ImageAlert update) {
        log.infof("Executing method %s", "updateSent");
        log.debugf("With parameters: %s", update.toString());
        String sql = UPDATE.concat(SET).concat(WHERE_ID);
        List<Object> parameters = new ArrayList<>();
        parameters.add(update.image());
        parameters.add(update.sent());
        parameters.add(update.topic());
        parameters.add(update.telegramAddress());
        parameters.add(update.id());
        this.coreRepository.execute(sql, parameters);
    }

    public void deleteSent() {
        log.infof("Executing method %s", "delete");
        String sql = DELETE.concat(WHERE_SENT);
        List<Object> parameters = new ArrayList<>();
        parameters.add(SENT);
        this.coreRepository.execute(sql, parameters);
    }
}
