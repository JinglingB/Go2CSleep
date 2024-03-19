package big.pimpin.go2sleephoe;

public final class DccActivity extends BaseSshActivity {
    public DccActivity() {
        super("C:\\Windows\\System32\\schtasks.exe /run /tn \"Switch Monitor Input\" >NUL 2>&1");
    }
}
