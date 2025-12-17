package alertador.videoportero.telegram.repository.sql;

public class ImageAlertSql {
    public static final String TABLE_NAME = "IMAGE_ALERT";
    public static final Integer ROW_ELEMENTS = 5;
    public static final String CREATE_TABLE = 
        "CREATE TABLE " + TABLE_NAME + " ("
        + " ID INTEGER PRIMARY KEY," 
        + " IMAGE BLOB NOT NULL," 
        + " TOPIC VARCHAR(50) NOT NULL,"
        + " TELEGRAM_ADDRESS VARCHAR(20) NOT NULL," 
        + " SENT INTEGER DEFAULT 0" 
        + ") ";
    public static final String INSERT = "INSERT INTO " + TABLE_NAME + " (IMAGE, TOPIC, TELEGRAM_ADDRESS, SENT) VALUES (?, ?, ?, ?) ";
    public static final String SELECT = "SELECT ID, IMAGE, TOPIC, TELEGRAM_ADDRESS, SENT ";
    public static final String FROM = "FROM " + TABLE_NAME + " ";
    public static final String UPDATE = "UPDATE " + TABLE_NAME + " ";
    public static final String SET = "SET IMAGE = ?, TOPIC = ?, TELEGRAM_ADDRESS = ?, SENT = ? ";
    public static final String DELETE = "DELETE FROM " + TABLE_NAME + " ";
    public static final String WHERE_ID = "WHERE ID = ? ";
    public static final String WHERE_SENT = "WHERE SENT = ? ";
}