open ReactNative;

let getPlatformStyle iosSize androidSize =>
  switch Platform.os {
  | IOS => iosSize
  | Android => androidSize
  };

let appBarHeight = getPlatformStyle 44. 56.;

let statusBarHeight = getPlatformStyle 20. 0.;

let titleOffset = getPlatformStyle 70. 56.;

let iosPlatformStyle =
  Style.[borderBottomWidth StyleSheet.hairlineWidth, borderBottomColor "rgba(0, 0, 0, .3)"];

let androidPlatformStyle =
  Style.[
    shadowColor "black",
    shadowOpacity 0.1,
    shadowRadius StyleSheet.hairlineWidth,
    elevation 4.
  ];

let platformContainerStyles = getPlatformStyle iosPlatformStyle androidPlatformStyle;

module ItemStyle = {
  let shared = Style.[justifyContent `center, alignItems `center, backgroundColor "transparent"];
  let styles =
    StyleSheet.create Style.({"right": style [right 0., bottom 0., top 0., position `absolute]});
};

module HeaderTitle = {
  let computedStyle ::position ::index => {
    let index = float index;
    Style.style [
      Style.opacityInterpolated @@
      Animated.Value.interpolate
        position
        inputRange::[index -. 1., index, index +. 1.]
        outputRange::(`float [0., 1., 0.])
        (),
      Style.transformInterpolated
        translateX::(
          Animated.Value.interpolate
            position
            inputRange::[index -. 1., index +. 1.]
            outputRange::(`float [200., (-200.)])
            ()
        )
        ()
    ]
  };
  let styles =
    StyleSheet.create
      Style.(
        {
          "titleContainer":
            style [
              bottom 0.,
              left titleOffset,
              right titleOffset,
              top 0.,
              position `absolute,
              alignItems @@ getPlatformStyle `center `flexStart,
              ...ItemStyle.shared
            ],
          "title":
            style [
              fontSize @@ getPlatformStyle 17.0 20.0,
              fontWeight @@ getPlatformStyle `_600 `_500,
              color "rgba(0, 0, 0, .9)",
              textAlign @@ getPlatformStyle `center `left
              /* marginHorizontal 16.0 */
            ]
        }
      );
  module IOS = {
    let render ::title ::position ::index => {
      let computed = computedStyle ::position ::index;
      <Animated.View style=Style.(concat [computed, styles##titleContainer])>
        <View style=styles##titleContainer>
          (
            ReasonReact.element @@
            Animated.Text.make value::title numberOfLines::1 style::styles##title [||]
          )
        </View>
      </Animated.View>
    };
  };
  module Android = {
    let render ::title =>
      <View style=styles##titleContainer>
        <View style=styles##titleContainer>
          (
            ReasonReact.element @@
            Animated.Text.make value::title numberOfLines::1 style::styles##title [||]
          )
        </View>
      </View>;
  };
};

module HeaderBackButton = {
  let computedStyle ::position ::index => {
    let index = float index;
    Style.style [
      Style.opacityInterpolated @@
      Animated.Value.interpolate
        position
        inputRange::[index -. 1., index -. 0.5, index, index +. 0.5, index +. 1.]
        outputRange::(`float [0., 0., 1., 0., 0.])
        ()
    ]
  };
  let iconIOS =
    Style.[
      height 21.,
      width 13.,
      marginLeft 10.,
      marginRight 22.,
      marginVertical 12.,
      resizeMode `contain
    ];
  let iconAndroid = Style.[height 24., width 24., margin 16., resizeMode `contain];
  let styles =
    StyleSheet.create
      Style.(
        {
          "container": style [alignItems `center, flexDirection `row, backgroundColor "transparent"],
          "iconWithTitle": getPlatformStyle (style [marginRight 5.]) (style []),
          "icon": Style.style @@ getPlatformStyle iconIOS iconAndroid,
          "containerLeft":
            style @@ ItemStyle.shared @ [left 0., bottom 0., top 0., position `absolute]
        }
      );
  let backButtonText text::_ => ReasonReact.nullElement;
  let render ::position ::index ::onPress => {
    let style = computedStyle ::position ::index;
    <Animated.View style=(Style.concat [style, styles##containerLeft])>
      <TouchableOpacity onPress style=styles##container>
        <View style=styles##container>
          <Image
            style=styles##icon
            source=(Required (Packager.require "../../../assets/back.png"))
          />
          (backButtonText text::"")
        </View>
      </TouchableOpacity>
    </Animated.View>
  };
};

let styles =
  StyleSheet.create
    Style.(
      {
        "header2": style [flexDirection `row],
        "header1": style [flex 1.],
        "header0":
          style [
            paddingTop statusBarHeight,
            backgroundColor @@ getPlatformStyle "#F7F7F7" "#FFF",
            height @@ statusBarHeight +. appBarHeight,
            ...platformContainerStyles
          ]
      }
    );

let renderHeader ::goBack ::position index title =>
  <View
    key=(string_of_int index ^ title)
    style=(Style.concat [StyleSheet.absoluteFill, styles##header2])>
    (HeaderTitle.IOS.render ::title ::position ::index)
    (
      if (index > 0 && Platform.os != Android) {
        HeaderBackButton.render ::position onPress::goBack ::index
      } else {
        ReasonReact.nullElement
      }
    )
  </View>;

module IOS = {
  let renderHeader ::goBack ::position index title =>
    <View
      key=(string_of_int index ^ title)
      style=(Style.concat [StyleSheet.absoluteFill, styles##header2])>
      (HeaderTitle.IOS.render ::title ::position ::index)
      (
        if (index > 0) {
          HeaderBackButton.render ::position onPress::goBack ::index
        } else {
          ReasonReact.nullElement
        }
      )
    </View>;
  let renderHeadersIOS ::goBack ::titles ::position =>
    ReasonReact.arrayToElement (
      Array.of_list @@ List.mapi (renderHeader ::goBack ::position) titles
    );
  let render ::position ::titles ::goBack =>
    <View style=styles##header0>
      <View style=styles##header1> (renderHeadersIOS ::goBack ::titles ::position) </View>
    </View>;
};

module Android = {
  let renderHeader title =>
    <View style=(Style.concat [StyleSheet.absoluteFill, styles##header2])>
      (HeaderTitle.Android.render ::title)
    </View>;
  let render ::title =>
    <View style=styles##header0> <View style=styles##header1> (renderHeader title) </View> </View>;
};
