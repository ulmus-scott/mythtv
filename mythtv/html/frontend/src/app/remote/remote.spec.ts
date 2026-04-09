import { ComponentFixture, TestBed } from '@angular/core/testing';

import { Remote } from './remote';

describe('Remote', () => {
  let component: Remote;
  let fixture: ComponentFixture<Remote>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [Remote]
    })
    .compileComponents();

    fixture = TestBed.createComponent(Remote);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
