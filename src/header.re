open BsReactNative;

let getPlatformStyle = (iosSize, androidSize) =>
  switch (Platform.os()) {
  | IOS(Phone) => iosSize
  | IOS(TV) => iosSize
  | IOS(Pad) => iosSize
  | Android => androidSize
  };

let appBarHeight = getPlatformStyle(44., 56.);

let statusBarHeight = getPlatformStyle(20., 0.);

let titleOffset = getPlatformStyle(70., 56.);

let iosPlatformStyle =
  Style.[
    borderBottomWidth(StyleSheet.hairlineWidth),
    borderBottomColor(String("rgba(0, 0, 0, .3)")),
  ];

let androidPlatformStyle =
  Style.[
    shadowColor(String("black")),
    shadowOpacity(0.1),
    shadowRadius(StyleSheet.hairlineWidth),
    elevation(4.),
  ];

let platformContainerStyles =
  getPlatformStyle(iosPlatformStyle, androidPlatformStyle);

module ItemStyle = {
  let shared =
    Style.[
      justifyContent(Center),
      alignItems(Center),
      backgroundColor(String("transparent")),
    ];
  let styles =
    StyleSheet.create(
      Style.(
        {
          "right":
            style([
              right(Pt(0.)),
              bottom(Pt(0.)),
              top(Pt(0.)),
              position(Absolute),
            ]),
        }
      ),
    );
};

module HeaderTitle = {
  let computedStyle = (~position, ~index) => {
    let index = float(index);
    Style.style([
      Style.opacity(
        Style.Animated(
          Animated.Value.interpolate(
            position,
            ~inputRange=[index -. 1., index, index +. 1.],
            ~outputRange=`float([0., 1., 0.]),
            (),
          ),
        ),
      ),
      Style.Transform.makeAnimated(
        ~translateX=
          Animated.Value.interpolate(
            position,
            ~inputRange=[index -. 1., index +. 1.],
            ~outputRange=`float([200., (-200.)]),
            (),
          ),
        (),
      ),
    ]);
  };
  let styles =
    StyleSheet.create(
      Style.(
        {
          "titleContainer":
            style([
              bottom(Pt(0.)),
              left(Pt(titleOffset)),
              right(Pt(titleOffset)),
              top(Pt(0.)),
              position(Absolute),
              alignItems @@
              getPlatformStyle(Center: Style.alignItems, FlexStart),
              ...ItemStyle.shared,
            ]),
          "title":
            style([
              fontSize @@ getPlatformStyle(Float(17.0), Float(20.0)),
              fontWeight @@ getPlatformStyle(`_600, `_500),
              color(String("rgba(0, 0, 0, .9)")),
              textAlign @@ getPlatformStyle(Center: Style.textAlign, Left),
              marginHorizontal(Pt(16.)),
            ]),
        }
      ),
    );
  module IOS = {
    let render = (~title, ~position, ~index) => {
      let computed = computedStyle(~position, ~index);
      <Animated.View style=Style.(concat([computed, styles##titleContainer]))>
        (
          ReasonReact.element @@
          Animated.Text.make(
            ~value=title,
            ~numberOfLines=1,
            ~style=styles##title,
            [||],
          )
        )
      </Animated.View>;
    };
  };
  module Android = {
    let render = (~title) =>
      <View style=styles##titleContainer>
        <View style=styles##titleContainer>
          (
            ReasonReact.element @@
            Animated.Text.make(
              ~value=title,
              ~numberOfLines=1,
              ~style=styles##title,
              [||],
            )
          )
        </View>
      </View>;
  };
};

module HeaderBackButton = {
  let computedStyle = (~position, ~index) => {
    let index = float(index);
    Style.style([
      Style.opacity @@
      Style.Animated(
        Animated.Value.interpolate(
          position,
          ~inputRange=[
            index -. 1.,
            index -. 0.5,
            index,
            index +. 0.5,
            index +. 1.,
          ],
          ~outputRange=`float([0., 0., 1., 0., 0.]),
          (),
        ),
      ),
    ]);
  };
  let iconIOS =
    Style.[
      height(Pt(21.)),
      width(Pt(13.)),
      marginLeft(Pt(10.)),
      marginRight(Pt(22.)),
      marginVertical(Pt(12.)),
      resizeMode(Contain),
    ];
  let iconAndroid =
    Style.[
      height(Pt(24.)),
      width(Pt(24.)),
      margin(Pt(16.)),
      resizeMode(Contain),
    ];
  let styles =
    StyleSheet.create(
      Style.(
        {
          "container":
            style([
              alignItems(Center),
              flexDirection(Row),
              backgroundColor(String("transparent")),
            ]),
          "iconWithTitle":
            getPlatformStyle(style([marginRight(Pt(5.))]), style([])),
          "icon": Style.style @@ getPlatformStyle(iconIOS, iconAndroid),
          "containerLeft":
            style @@
            ItemStyle.shared
            @ [
              left(Pt(0.)),
              bottom(Pt(0.)),
              top(Pt(0.)),
              position(Absolute),
            ],
        }
      ),
    );
  let backButtonText = (~text as _) => ReasonReact.null;
  let render = (~position, ~index, ~onPress) => {
    let style = computedStyle(~position, ~index);
    <Animated.View style=(Style.concat([style, styles##containerLeft]))>
      <TouchableOpacity onPress style=styles##container>
        <View style=styles##container>
          <Image
            style=styles##icon
            source=`Required(Packager.require("../../../assets/back.png"))
          />
          (backButtonText(~text=""))
        </View>
      </TouchableOpacity>
    </Animated.View>;
  };
};

let styles =
  StyleSheet.create(
    Style.(
      {
        "header2": style([flexDirection(Row)]),
        "header1": style([flex(1.)]),
        "header0":
          style([
            paddingTop(Pt(statusBarHeight)),
            backgroundColor @@
            String(getPlatformStyle("#F7F7F7", "#FFF")),
            height @@ Pt(statusBarHeight +. appBarHeight),
            ...platformContainerStyles,
          ]),
      }
    ),
  );

let renderHeader = (~goBack, ~position, index, title) =>
  <View
    key=(string_of_int(index) ++ title)
    style=(Style.concat([StyleSheet.absoluteFill, styles##header2]))>
    (HeaderTitle.IOS.render(~title, ~position, ~index))
    (
      if (index > 0 && Platform.os() != Android) {
        HeaderBackButton.render(~position, ~onPress=goBack, ~index);
      } else {
        ReasonReact.null;
      }
    )
  </View>;

module IOS = {
  let renderHeader = (~goBack, ~position, index, title) =>
    <View
      key=(string_of_int(index) ++ title)
      style=(Style.concat([StyleSheet.absoluteFill, styles##header2]))>
      (HeaderTitle.IOS.render(~title, ~position, ~index))
      (
        if (index > 0) {
          HeaderBackButton.render(~position, ~onPress=goBack, ~index);
        } else {
          ReasonReact.null;
        }
      )
    </View>;
  let renderHeadersIOS = (~goBack, ~titles, ~position) =>
    ReasonReact.array(
      Array.of_list @@ List.mapi(renderHeader(~goBack, ~position), titles),
    );
  let render = (~position, ~titles, ~goBack) =>
    <View style=styles##header0>
      <View style=styles##header1>
        (renderHeadersIOS(~goBack, ~titles, ~position))
      </View>
    </View>;
};

module Android = {
  let renderHeader = title =>
    <View style=(Style.concat([StyleSheet.absoluteFill, styles##header2]))>
      (HeaderTitle.Android.render(~title))
    </View>;
  let render = (~title) =>
    <View style=styles##header0>
      <View style=styles##header1> (renderHeader(title)) </View>
    </View>;
};
