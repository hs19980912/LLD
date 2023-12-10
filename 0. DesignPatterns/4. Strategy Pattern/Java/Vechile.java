public class Vehicle{
    DriveStrategy strategy;

    // This is called constructor injection.
    Vehicle(DriveStrategy strategy){
        this.strategy = strategy;
    }
    public void drive(){
        strategy.drive();
    }
}
