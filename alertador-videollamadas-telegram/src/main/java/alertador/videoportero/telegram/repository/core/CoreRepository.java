package alertador.videoportero.telegram.repository.core;

import org.eclipse.microprofile.config.inject.ConfigProperty;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;

import java.util.List;
import java.util.stream.Collectors;
import java.util.ArrayList;
import java.util.Arrays;

import jakarta.annotation.PostConstruct;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;

import org.jboss.logging.Logger;

@ApplicationScoped
public class CoreRepository {

    public static final String CHECK_TABLE = "SELECT name FROM sqlite_master WHERE type='table' AND name=?";
    public static final String MAX_ID = "SELECT MAX(ID) FROM ";
    public static final String DROP_TABLE = "DROP TABLE ";
    public static final String DROP_CREATE_ACTION = "DROP_CREATE";
    public static final String CREATE_ACTION = "CREATE";

    private static final String SEPARATOR = "/";

    @ConfigProperty(name="quarkus.datasource.sqlite.file")
    private String sqliteFile;

    @ConfigProperty(name = "quarkus.datasource.jdbc.url")
    private String jdbcUrl;

    private Logger log;

    @Inject
    public CoreRepository(Logger log) {
        this.log = log;
    }

    @PostConstruct
    private void checkDataBaseExistence() {
        var path = Paths.get(sqliteFile);
        if (!Files.exists(path)) {
            var pathParts = sqliteFile.split("/");
            var filePath = List.of(Arrays.copyOfRange(pathParts, 0, pathParts.length - 1)).stream().collect(Collectors.joining(SEPARATOR));
            var dbDir = new File(SEPARATOR.concat(filePath));
            dbDir.mkdir();
            try {
                new File(dbDir, pathParts[pathParts.length-1]).createNewFile();
            } catch (IOException e) {
                log.errorf("Error at creating: %s", sqliteFile);
            }
        }
    }

    public boolean exist(String sql, List<Object> parameters) {
        log.infof("Executing method %s", "exists");
        log.debugf("With sql sentence %s and parameters %s", sql, parameters);
        try (var conn = DriverManager.getConnection(jdbcUrl); var stmt = conn.prepareStatement(sql)) {
            setParameters(stmt, parameters);
            return stmt.executeQuery().next();            
        } catch (SQLException e) {
            log.errorf("Error at method %s : %s", "exists", e.getMessage());
        }
        return false;
    }

    public boolean execute(String sql, List<Object> parameters) {
        log.infof("Executing method %s", "execute");
        log.debugf("With sql sentence %s and parameters %s", sql, parameters);
        try (var conn = DriverManager.getConnection(jdbcUrl); var stmt = conn.prepareStatement(sql)) {
            setParameters(stmt, parameters);
            stmt.execute();
            log.infof("%s done!", "execute");
        } catch (SQLException e) {
            log.errorf("Error at method %s : %s", "execute", e.getMessage());
            return false;
        }
        return true;
    }

    public List<List<Object>> select(String sql, List<Object> parameters, Integer rowElements) {
        log.infof("Executing method %s", "select");
        log.debugf("With sql sentence %s and parameters %s", sql, parameters);
        List<List<Object>> result = new ArrayList<>();
        try (var conn = DriverManager.getConnection(jdbcUrl); var stmt = conn.prepareStatement(sql)) {
            setParameters(stmt, parameters);
            var rs = stmt.executeQuery();
            List<Object> row;
            while (rs.next()) {
                row = new ArrayList<>();
                int i = 1;
                while (i <= rowElements) {
                    log.debug("Next part");
                    row.add(rs.getObject(i++));
                }
                log.debug("Next row");
                result.add(row);
            }
            rs.close();
            log.debugf("Result: %s", result);
            return result;
        } catch (SQLException e) {
            log.errorf("Error at method %s : %s", "select", e.getMessage());
            return null;
        }
    }

    private void setParameters(PreparedStatement stmt, List<Object> parameters) {
        log.infof("Executing method %s", "setParameters");
        log.debugf("With parameters %s", parameters);
        var i = 1;
        try {
            for (var parameter : parameters) {
                if (parameter instanceof String) {
                    log.debugf("Setting string parameter: %s", (String) parameter);
                    stmt.setString(i++, (String) parameter);
                } else if (parameter instanceof Integer) {
                    log.debugf("Setting integer parameter: %s", (Integer) parameter);
                    stmt.setInt(i++, (Integer) parameter);
                } else if (parameter instanceof byte []) {
                    log.debugf("Setting byte [] parameter: %s", (byte []) parameter);
                    stmt.setBytes(i++, (byte[]) parameter);
                }
            }
        } catch (SQLException e) {
            log.errorf("Error at method %s : %s", "setParameters", e.getMessage());
        }
    }
}