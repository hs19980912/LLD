public class Vechile{
    DriveStrategy strategy;

    // This is called constructor injection.
    Vechile(DriveStrategy strategy){
        this.strategy = strategy;
    }
    public void drive(){
        strategy.drive();
    }
}