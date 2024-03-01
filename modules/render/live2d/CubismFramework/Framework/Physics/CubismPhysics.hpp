﻿/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include "Math/CubismVector2.hpp"
#include "CubismPhysicsInternal.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismModel;
struct CubismPhysicsRig;

/**
 * @brief 物理演算クラス
 *
 * 物理演算のクラス。
 */
class CubismPhysics
{
public:
    /**
     * @brief オプション
     *
     * 物理演算のオプション。
     */
    struct Options
    {
        CubismVector2 Gravity; ///< 重力方向
        CubismVector2 Wind; ///< 風の方向
    };

    /**
     * @brief 物理演算出力結果
     *
     * パラメータに適用する前の物理演算の出力結果
     */
    struct PhysicsOutput
    {
        csmVector<csmFloat32> output;
    };

    /**
     * @brief インスタンスの作成
     *
     * インスタンスを作成する。
     *
     * @param[in]   buffer      physics3.jsonが読み込まれいるバッファ
     * @param[in]   size        バッファのサイズ
     * @return  作成されたインスタンス
     */
    static CubismPhysics* Create(const csmByte* buffer, csmSizeInt size);

    /**
     * @brief インスタンスの破棄
     *
     * インスタンスを破棄する。
     *
     * @param[in]   physics     破棄するインスタンス
     */
    static void Delete(CubismPhysics* physics);

    /**
     * @brief パラメータのリセット
     *
     * パラメータをリセットする。
     */
    void Reset();

    /**
     * @brief 物理演算の評価
     *
     * 物理演算を評価する。
     *
     * @param[in]   model               物理演算の結果を適用するモデル
     * @param[in]   deltaTimeSeconds    デルタ時間[秒]
     */
    void Evaluate(CubismModel* model, csmFloat32 deltaTimeSeconds);

    /**
     * @brief オプションの設定
     *
     * オプションを設定する。
     *
     * @param[in]   options     オプション
     */
    void SetOptions(const Options& options);

    /**
     * @brief オプションの取得
     *
     * オプションを取得する。
     *
     * @return オプション
     */
    const Options& GetOptions() const;

private:
    /**
     * @brief コンストラクタ
     *
     * コンストラクタ。
     */
    CubismPhysics();

    /**
     * @brief デストラクタ
     *
     * デストラクタ。
     */
    virtual ~CubismPhysics();

    // Prevention of copy Constructor
    CubismPhysics(const CubismPhysics&);
    CubismPhysics& operator=(const CubismPhysics&);

    /**
     * @brief physics3.jsonのパース
     *
     * physics3.jsonをパースする。
     *
     * @param[in]   physicsJson     physics3.jsonが読み込まれているバッファ
     * @param[in]   size            バッファのサイズ
     */
    void Parse(const csmByte* physicsJson, csmSizeInt size);

    /**
     * @brief 初期化
     *
     * 初期化する。
     */
    void Initialize();

    /**
     * @brief 物理演算結果の適用
     *
     * 振り子演算の最新の結果と一つ前の結果から指定した重みで適用する。
     *
     * @param model 物理演算の結果を適用するモデル
     * @param weight 最新結果の重み
     */
    void Interpolate(CubismModel* model, csmFloat32 weight);

    CubismPhysicsRig* _physicsRig; ///< 物理演算のデータ
    Options _options; ///< オプション

    csmVector<PhysicsOutput> _currentRigOutputs; ///< 最新の振り子計算の結果
    csmVector<PhysicsOutput> _previousRigOutputs; ///< 一つ前の振り子計算の結果

    csmFloat32 _currentRemainTime; ///< 物理演算が処理していない時間

    csmVector<csmFloat32> _parameterCache;      ///< Evaluateで利用するパラメータのキャッシュ
    csmVector<csmFloat32> _parameterInputCache; ///< UpdateParticlesが動くときの入力をキャッシュ

    csmBool _isJsonValid; ///< 正しくJsonデータが取得出来たか
};

}}}
