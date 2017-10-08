/* module type Impl = {type navigationScreenType; let initialScreen: navigationScreenType;};

   type action =
     | Pop
     | Push Impl.navigationState
     | Increment
     | Decrement;

   type state = {
     navigationState: list screen,
     count: int
   };

   let component = ReasonReact.reducerComponent "App"; */
