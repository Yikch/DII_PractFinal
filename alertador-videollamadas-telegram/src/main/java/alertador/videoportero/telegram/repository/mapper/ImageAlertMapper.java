package alertador.videoportero.telegram.repository.mapper;

import alertador.videoportero.telegram.dto.ImageAlert;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;

import java.util.List;

import org.jboss.logging.Logger;

import java.util.ArrayList;

@ApplicationScoped
public class ImageAlertMapper {
    private Logger log;
    
    @Inject
    public ImageAlertMapper(Logger log) {
       this.log = log; 
    }

    public List<ImageAlert> map(List<List<Object>> preresult) {
        List<ImageAlert> result = new ArrayList<>();
        int i = 0;
        for (var row : preresult) {
            i = 0;
            result.add(
                new ImageAlert(
                    (Integer) row.get(i++),
                    (byte []) row.get(i++), 
                    (String) row.get(i++),
                    (String) row.get(i++),
                    (Integer) row.get(i)
                )
            );
        } 
        log.debugf("Result: %s", result);
        return result;
    }
}