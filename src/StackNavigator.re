module type Impl = {type navigationState;};

open BsReactNative;

module Make = (Impl: Impl) => {
  type state = {
    width: float,
    position: Animated.Value.t,
    onScreenNavigationState: list(Impl.navigationState),
    onBackAndroid: option(unit => bool)
  };
  type action =
    | SetBackHandler
    | UpdateWidth(float)
    | UpdateOnScreenState(
        list(Impl.navigationState),
        list(Impl.navigationState)
      );
  let animationDuration = 300.;
  let positionThreshold = 0.5;
  let responseThreshold = 20.;
  let gestureDistanceHorizontal = 75;
  let gestureDistanceVertical = 135;
  let computedStyle = (~position, ~index, ~width, ~length) => {
    let intIndex = index;
    let index = float(index);
    let commonStyle =
      Style.[
        flex(1.),
        backgroundColor(String("#E9E9EF")),
        bottom(Pt(0.)),
        left(Pt(0.)),
        Style.position(Absolute),
        right(Pt(0.)),
        shadowColor(String("black")),
        shadowOffset(~width=0., ~height=0.),
        shadowOpacity(0.2),
        shadowRadius(5.),
        top(Pt(0.))
      ];
    switch (Platform.os()) {
    | IOS(_) =>
      Style.style([
        Style.opacity @@
        Style.Animated(
          Animated.Value.interpolate(
            position,
            ~inputRange=[
              index -. 1.,
              index -. 0.99,
              index,
              index +. 0.99,
              index +. 1.,
            ],
            ~outputRange=`float([0., 1., 1., 0.85, 0.]),
            ()
          )
        ),
        if (intIndex == length - 1) {
          Style.Transform.makeAnimated(
            ~translateX=
              Animated.Value.interpolate(
                position,
                ~inputRange=[index -. 1., index],
                ~outputRange=`float([width, 0.]),
                ()
              ),
            ()
          );
        } else {
          Style.Transform.makeAnimated(
            ~translateX=
              Animated.Value.interpolate(
                position,
                ~inputRange=[index, index +. 1.],
                ~outputRange=`float([0., (-150.)]),
                ()
              ),
            ()
          );
        },
        ...commonStyle
      ])
    | Android =>
      Style.style([
        Style.opacity @@
        Animated(
          Animated.Value.interpolate(
            position,
            ~inputRange=[index -. 1., index, index +. 0.99, index +. 1.],
            ~outputRange=`float([0., 1., 1., 0.]),
            ()
          )
        ),
        Style.Transform.makeAnimated(
          ~translateY=
            Animated.Value.interpolate(
              position,
              ~inputRange=[index -. 1., index, index +. 0.99, index +. 1.],
              ~outputRange=`float([50., 0., 0., 0.]),
              ()
            ),
          ()
        ),
        ...commonStyle
      ])
    }
  };
  let renderCard = (~handlers, ~position, ~width, ~length, index, screen) => {
    let computed = computedStyle(~width, ~position, ~index, ~length);
    <Animated.View
      responderHandlers=handlers key=(string_of_int(index)) style=computed
    >
      screen
    </Animated.View>
  };
  let renderCardStack = (~handlers, ~position, ~screens, ~width) => {
    <View style=Style.(style([flex(1.)]))>
      (
        ReasonReact.array @@
        Array.of_list @@
        List.mapi(
          renderCard(
            ~handlers,
            ~position,
            ~width,
            ~length=List.length(screens)
          ),
          screens
        )
      )
    </View>
  };
  let renderIOS = (~handlers, ~width, ~position, ~screens, ~titles, ~goBack) => {
    <View style=Style.(style([flexDirection(Column), flex(1.)]))>
      (Header.IOS.render(~position, ~goBack, ~titles))
      (renderCardStack(~handlers, ~width, ~position, ~screens))
    </View>
  };
  let renderCardAndroid = (~handlers, ~position, ~width, index, screen, ~titles) => {
    let computed = computedStyle(~width, ~position, ~index, ~length=0);
    let title = List.nth(titles, index);
    <Animated.View
      responderHandlers=handlers key=(string_of_int(index)) style=computed
    >
      (Header.Android.render(~title))
      screen
    </Animated.View>
  };
  let renderAndroid = (~handlers, ~width, ~position, ~screens, ~titles) => {
    <View style=Style.(style([flex(1.)]))>
      (
        ReasonReact.array @@
        Array.of_list @@
        List.mapi(
          renderCardAndroid(~handlers, ~position, ~width, ~titles),
          screens
        )
      )
    </View>
  };
  let animateScreen =
      (
        ~duration,
        ~callback=?,
        ~position,
        ~index,
        ~toValue=`raw(float @@ index),
        ()
      ) =>
    Animated.(
      start(
        Value.Timing.(animate(~duration, ~value=position, ~toValue, ())),
        ~callback?,
        ()
      )
    );
  let reset = (~resetToIndex, ~duration, ~position) =>
    animateScreen(~duration, ~position, ~index=resetToIndex, ());
  let goBackCard = (~pop, ~backFromIndex, ~duration, ~position) => {
    let toValue = `raw(float @@ max(backFromIndex - 1, 0));
    let callback = (_) => {
      let _ = pop();
      ()
    };
    animateScreen(
      ~callback,
      ~toValue,
      ~duration,
      ~position,
      ~index=backFromIndex,
      ()
    )
  };
  let backAndroid = (goBack, _) => {
    let _ = goBack();
    true;
  };
  let component = ReasonReact.reducerComponent("AppNavigator");
  let make =
    (
      ~navigationState: list(Impl.navigationState),
      ~render,
      ~getHeaderConfig,
      ~goBack
    ) => {
    ...component,
    initialState: () => {
      onBackAndroid: None,
      width: 0.,
      position: Animated.Value.create(0.),
      onScreenNavigationState: navigationState
    },
    reducer: (action, state) =>
      switch (action) {
      | SetBackHandler => ReasonReact.Update({...state, onBackAndroid: Some(backAndroid(goBack))})
      | UpdateWidth(width) => ReasonReact.Update({...state, width})
      | UpdateOnScreenState(nextState, currentState) =>
        if (currentState === state.onScreenNavigationState) {
          ReasonReact.Update({...state, onScreenNavigationState: nextState})
        } else {
          ReasonReact.NoUpdate
        }
      },
    didMount: (self) => {
      self.send(SetBackHandler);
      BackHandler.addEventListener(
        "hardwareBackPressReasonNative",
        backAndroid(goBack)
      )
    },
    didUpdate: ({oldSelf: {state: {onScreenNavigationState}}, newSelf: {state: {position}, send}}) => {
      let oldLength = List.length(onScreenNavigationState);
      let newLength = List.length(navigationState);
      if (oldLength == newLength) {
        ()
      } else {
        let callback =
          oldLength <= newLength ?
            None :
            Some(
              (_) => send(
                UpdateOnScreenState(
                  navigationState,
                  onScreenNavigationState
                )
              )
            );
        animateScreen(
          ~duration=animationDuration,
          ~callback?,
          ~position,
          ~index=newLength - 1,
          ()
        )
      }
    },
    willReceiveProps: ({state}) => {
      let oldLength = List.length(state.onScreenNavigationState);
      let newLength = List.length(navigationState);
      if (oldLength < newLength) {
        {...state, onScreenNavigationState: navigationState}
      } else if (oldLength > newLength) {
        state
      } else if (
        List.fold_left2(
          (acc, x1, x2) => acc && x1 == x2,
          true,
          state.onScreenNavigationState,
          navigationState
        )
      ) {
        state
      } else {
        {...state, onScreenNavigationState: navigationState}
      }
    },
    willUnmount: ({state}) =>
      switch (state.onBackAndroid) {
      | Some(backAndroid) =>
        BackHandler.removeEventListener(
          "hardwareBackPressReasonNative",
          backAndroid
        )
      | _ => ()
      },
    render: ({state, send}) => {
      let {position, onScreenNavigationState, width} = state;
      let index = List.length(onScreenNavigationState) - 1;
      let panResponder =
        PanResponder.create(
          ~onPanResponderTerminate=
            PanResponder.callback @@
            (
              (_, _) =>
                reset(
                  ~resetToIndex=index,
                  ~duration=animationDuration,
                  ~position
                )
            ),
          ~onMoveShouldSetPanResponder=
            PanResponder.callback @@
            (
              (event, gesture) => {
                let pageX = RNEvent.NativeEvent.pageX(event);
                /* let pageY = RNEvent.NativeEvent.pageY(event); */
                let currentDragDistance = gesture.dx;
                let currentDragPosition = pageX;
                let screenEdgeDistance = currentDragPosition -. currentDragDistance;
                let gestureResponseDistance = float @@ gestureDistanceHorizontal;
                if (screenEdgeDistance > gestureResponseDistance) {
                  false
                } else {
                  let hasDraggedEnough = abs_float(currentDragDistance) > responseThreshold;
                  let isOnFirstCard = index == 0;
                  hasDraggedEnough && ! isOnFirstCard
                }
              }
            ),
          ~onPanResponderMove=
            `callback(
              PanResponder.callback @@
              (
                (_, gesture) => {
                  let startValue = float(index);
                  let axisDistance = state.width;
                  if (gesture.dx > 0.) {
                    let currentValue = (startValue -. gesture.dx) /. axisDistance;
                    let value = float(index) +. currentValue;
                    AnimatedRe.Value.setValue(position, value)
                  }
                }
              )
            ),
          ~onPanResponderTerminationRequest=
            PanResponder.callback @@ ((_, _) => false),
          ~onPanResponderRelease=
            PanResponder.callback @@
            (
              (_, gesture) => {
                let axisDistance = state.width;
                let movedDistance = gesture.dx;
                let gestureVelocity = gesture.vx;
                let defaultVelocity = axisDistance /. animationDuration;
                let velocity = max(abs_float(gestureVelocity), defaultVelocity);
                let resetDuration = movedDistance /. velocity;
                let goBackDuration = (axisDistance -. movedDistance) /. velocity;
                if (gestureVelocity < (-0.5)) {
                  reset(
                    ~resetToIndex=index,
                    ~duration=resetDuration,
                    ~position
                  )
                } else if (gestureVelocity > 1.) {
                  goBackCard(
                    ~pop=goBack,
                    ~backFromIndex=index,
                    ~duration=goBackDuration,
                    ~position
                  )
                } else if (movedDistance > axisDistance /. 2.) {
                  goBackCard(
                    ~pop=goBack,
                    ~backFromIndex=index,
                    ~duration=goBackDuration,
                    ~position
                  )
                } else {
                  reset(
                    ~resetToIndex=index,
                    ~duration=resetDuration,
                    ~position
                  )
                }
              }
            ),
          ()
        );
      switch (onScreenNavigationState) {
      | [] => ReasonReact.null
      | _ =>
        let f =
          switch (Platform.os()) {
          | IOS(_) => renderIOS(~goBack)
          | _ => renderAndroid
          };
        <View
          onLayout={e => send(UpdateWidth(RNEvent.NativeLayoutEvent.layout(e).width))}
          style=Style.(style([flex(1.)]))>
          (
            f(
              ~handlers=PanResponder.panHandlers(panResponder),
              ~width,
              ~position,
              ~screens=List.rev_map(render, onScreenNavigationState),
              ~titles=List.rev_map(getHeaderConfig, onScreenNavigationState)
            )
          )
        </View>
      }
    }
  };
};
