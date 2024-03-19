package big.pimpin.go2sleephoe;

public final class MainActivity extends BaseSshActivity {
    public MainActivity() {
        super("C:\\ProgramData\\scoop\\apps\\sysinternals\\current\\psshutdown64.exe -accepteula -nobanner -d -t 0 >NUL 2>&1");
    }
}
