package in.reconv.oboenativemodule;

public class NativeLib {

    // Used to load the 'oboenativemodule' library on application startup.
    static {
        System.loadLibrary("oboenativemodule");
    }

    /**
     * A native method that is implemented by the 'oboenativemodule' native library,
     * which is packaged with this application.
     */
    //public native String stringFromJNI();
    public static void letsdosomting(){
        System.out.println("Printidsfbaskjbaskdhasd");
    }
}