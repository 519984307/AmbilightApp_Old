// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';



export class InnerStruct implements flatbuffers.IUnpackableObject<InnerStructT> {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):InnerStruct {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

a():number {
  return this.bb!.readFloat64(this.bb_pos);
}

b(index: number):number|null {
    return this.bb!.readUint8(this.bb_pos + 8 + index);
}

c():number {
  return this.bb!.readInt8(this.bb_pos + 21);
}

dUnderscore():bigint {
  return this.bb!.readInt64(this.bb_pos + 24);
}

static getFullyQualifiedName():string {
  return 'MyGame.Example.InnerStruct';
}

static sizeOf():number {
  return 32;
}

static createInnerStruct(builder:flatbuffers.Builder, a: number, b: number[]|null, c: number, d_underscore: bigint):flatbuffers.Offset {
  builder.prep(8, 32);
  builder.writeInt64(BigInt(d_underscore ?? 0));
  builder.pad(2);
  builder.writeInt8(c);

  for (let i = 12; i >= 0; --i) {
    builder.writeInt8((b?.[i] ?? 0));

  }

  builder.writeFloat64(a);
  return builder.offset();
}


unpack(): InnerStructT {
  return new InnerStructT(
    this.a(),
    this.bb!.createScalarList<number>(this.b.bind(this), 13),
    this.c(),
    this.dUnderscore()
  );
}


unpackTo(_o: InnerStructT): void {
  _o.a = this.a();
  _o.b = this.bb!.createScalarList<number>(this.b.bind(this), 13);
  _o.c = this.c();
  _o.dUnderscore = this.dUnderscore();
}
}

export class InnerStructT implements flatbuffers.IGeneratedObject {
constructor(
  public a: number = 0.0,
  public b: (number)[] = [],
  public c: number = 0,
  public dUnderscore: bigint = BigInt('0')
){}


pack(builder:flatbuffers.Builder): flatbuffers.Offset {
  return InnerStruct.createInnerStruct(builder,
    this.a,
    this.b,
    this.c,
    this.dUnderscore
  );
}
}
